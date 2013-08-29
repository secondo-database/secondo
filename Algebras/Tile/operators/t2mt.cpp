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
system includes

*/

#include <cmath>

/*
TileAlgebra includes

*/

#include "t2mt.h"
#include "../grid/tgrid.h"
#include "../Index/Index.h"
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
Template method t2mtFunction implements the t2mt operator functionality.

author: Dirk Zacher
parameters: pArguments - a pointer to the arguments of t2mt operator
            rResult - reference to a Word containing the result
            message - message to distinguish call modes of t2mtFunction
            rLocal - reference to a Word to store local method information
            supplier - an Address to a supplier of information of operator tree
return value: 0 if t2mtFunction successfully executed, otherwise FAILURE
exceptions: -

*/

template <typename Type, typename Properties>
int t2mtFunction(Word* pArguments,
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
    datetime::DateTime* pDuration = static_cast<datetime::DateTime*>
                                    (pArguments[1].addr);
    Instant* pInstant1 = static_cast<Instant*>(pArguments[2].addr);
    Instant* pInstant2 = static_cast<Instant*>(pArguments[3].addr);

    if(pType != 0 &&
       pDuration != 0 &&
       pInstant1 != 0 &&
       pInstant2 != 0)
    {
      rResult = qp->ResultStorage(supplier);

      if(rResult.addr != 0)
      {
        typename Properties::PropertiesType* pResult =
        static_cast<typename Properties::PropertiesType*>(rResult.addr);

        if(pResult != 0)
        {
          pResult->SetDefined(false);

          if(pType->IsDefined() &&
             pDuration->IsDefined() &&
             pInstant1->IsDefined() &&
             pInstant2->IsDefined())
          {
            int tDimensionSize = Properties::GetTDimensionSize();
            int startTime = static_cast<int>
                            (round(pInstant1->ToDouble() /
                                   pDuration->ToDouble()));
            int endTime = static_cast<int>
                          (round(pInstant2->ToDouble() /
                                 pDuration->ToDouble()));

            if(startTime >= 0 &&
               startTime < tDimensionSize &&
               endTime >= 0 &&
               endTime < tDimensionSize)
            {
              pResult->SetDefined(true);

              tgrid grid;
              pType->getgrid(grid);

              bool bOK = pResult->SetGrid(grid.GetX(),
                                          grid.GetY(),
                                          grid.GetLength(),
                                          *pDuration);

              if(bOK == true)
              {
                Index<2> minimumIndex;
                Index<2> maximumIndex;
                bool bOK = pType->GetBoundingBoxIndexes(minimumIndex,
                                                        maximumIndex);

                if(bOK == true)
                {
                  for(int time = startTime; time <= endTime; time++)
                  {
                    for(int row = minimumIndex[1]; row < maximumIndex[1];
                        row++)
                    {
                      for(int column = minimumIndex[0];
                          column < maximumIndex[0]; column++)
                      {
                        typename Properties::TypeProperties::PropertiesType
                        value = Properties::TypeProperties::GetUndefinedValue();

                        Index<2> index2((int[]){column, row});
                        value = pType->GetValue(index2);

                        if(Properties::TypeProperties::IsUndefinedValue(value)
                           == false)
                        {
                          Index<3> index3((int[]){column, row, time});
                          bOK = pResult->SetValue(index3, value, true);
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
definition of t2mtFunctions array.

*/

ValueMapping t2mtFunctions[] =
{
  t2mtFunction<tint, mtProperties<int> >,
  t2mtFunction<treal, mtProperties<double> >,
  t2mtFunction<tbool, mtProperties<char> >,
  t2mtFunction<tstring, mtProperties<string> >,
  0
};

/*
Method t2mtSelectFunction returns the index of specific t2mt function
in t2mtFunctions array depending on the arguments.

author: Dirk Zacher
parameters: arguments - arguments of t2mt operator
return value: index of specific t2mt function in t2mtFunctions
exceptions: -

*/

int t2mtSelectFunction(ListExpr arguments)
{
  int functionIndex = -1;

  if(arguments != 0)
  {
    NList argumentsList(arguments);

    if(argumentsList.hasLength(4))
    {
      NList argument1 = argumentsList.first();
      NList argument2 = argumentsList.second();
      NList argument3 = argumentsList.third();
      NList argument4 = argumentsList.fourth();

      if(argument2.isSymbol(Duration::BasicType()) &&
         argument3.isSymbol(Instant::BasicType()) &&
         argument4.isSymbol(Instant::BasicType()))
      {
        const int TYPE_NAMES = 4;
        const std::string TYPE_NAMES_ARRAY[TYPE_NAMES] =
        {
          tint::BasicType(),
          treal::BasicType(),
          tbool::BasicType(),
          tstring::BasicType()
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
Method t2mtTypeMappingFunction returns the return value type
of t2mt operator in the form of a ListExpr.

author: Dirk Zacher
parameters: arguments - arguments of t2mt operator
return value: return value type of t2mt operator
exceptions: -

*/

ListExpr t2mtTypeMappingFunction(ListExpr arguments)
{
  ListExpr type = NList::typeError("Operator t2mt expects "
                                   "a t type, a duration, "
                                   "an instant and an instant.");

  NList argumentsList(arguments);

  if(argumentsList.hasLength(4))
  {
    std::string argument1 = argumentsList.first().str();
    std::string argument2 = argumentsList.second().str();
    std::string argument3 = argumentsList.third().str();
    std::string argument4 = argumentsList.fourth().str();

    if(IstType(argument1) &&
       argument2 == Duration::BasicType() &&
       argument3 == Instant::BasicType() &&
       argument4 == Instant::BasicType())
    {
      type = NList(GetmtType(argument1)).listExpr();
    }
  }

  return type;
}

}
