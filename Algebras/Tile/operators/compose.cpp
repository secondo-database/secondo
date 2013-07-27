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

#include "compose.h"
#include "../Index/Index.h"
#include "../t/tCellIterator.h"
#include "../t/tint.h"
#include "../t/treal.h"
#include "../t/tbool.h"
#include "../t/tstring.h"
#include "../mt/mtCellIterator.h"
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
Template method composeFunctiont implements the compose operator functionality
for t datatypes.

author: Dirk Zacher
parameters: pArguments - a pointer to the arguments of compose operator
            rResult - reference to a Word containing the result
            message - message to distinguish call modes of composeFunctiont
            rLocal - reference to a Word to store local method information
            supplier - an Address to a supplier of information of operator tree
return value: 0 if composeFunctiont successfully executed, otherwise FAILURE
exceptions: -

*/

template <typename Type, typename Properties>
int composeFunctiont(Word* pArguments,
                     Word& rResult,
                     int message,
                     Word& rLocal,
                     Supplier supplier)
{
  int nRetVal = FAILURE;

  if(qp != 0 &&
     pArguments != 0)
  {
    MPoint* pMPoint = static_cast<MPoint*>(pArguments[0].addr);
    Type* pType = static_cast<Type*>(pArguments[1].addr);

    if(pMPoint != 0 &&
       pType != 0)
    {
      rResult = qp->ResultStorage(supplier);

      if(rResult.addr != 0)
      {
        typename Properties::TypeProperties::MType* pResult =
        static_cast<typename Properties::TypeProperties::MType*>(rResult.addr);

        if(pResult != 0)
        {
          pResult->SetDefined(false);

          if(pMPoint->IsDefined() &&
             pType->IsDefined())
          {
            pResult->SetDefined(true);
            pResult->StartBulkLoad();

            int components = pMPoint->GetNoComponents();

            for(int i = 0; i < components; i++)
            {
              UPoint unit(0);
              pMPoint->Get(i, unit);

              double xStart = unit.p0.GetX();
              double yStart = unit.p0.GetY();
              double xEnd = unit.p1.GetX();
              double yEnd = unit.p1.GetY();

              if(pType->IsValidLocation(xStart, yStart) &&
                 pType->IsValidLocation(yEnd, yEnd))
              {
                Index<2> startIndex = pType->GetLocationIndex(xStart, yStart);
                Index<2> endIndex = pType->GetLocationIndex(xEnd, yEnd);

                if(startIndex == endIndex)
                {
                  typename Properties::TypeProperties::WrapperType value;
                  pType->atlocation(xStart, yStart, value);

                  if(value.IsDefined())
                  {
                    pResult->MergeAdd(typename Properties::TypeProperties::
                                      UnitType(unit.timeInterval,
                                               value, value));
                  }
                }

                else
                {
                  tCellIterator<Type> cellIterator(*pType,
                                                   xStart, yStart,
                                                   xEnd, yEnd);
                  double dx = xEnd - xStart;
                  double dy = yEnd - yStart;

                  DateTime startTime = unit.timeInterval.start;
                  DateTime endTime = unit.timeInterval.end;
                  DateTime duration = endTime - startTime;

                  while(cellIterator.HasNext())
                  {
                    pair<double,double> p = cellIterator.Next();
                    DateTime currentStartTime = startTime +
                                                (duration * p.first);
                    DateTime currentEndTime = startTime +
                                              (duration * p.second);

                    if(currentEndTime > currentStartTime)
                    {
                      Interval<Instant> interval(currentStartTime,
                                                 currentEndTime,
                                                 true, false);
                      double delta  =(p.first + p.second) / 2.0;
                      double x = xStart + delta * dx;
                      double y = yStart + delta * dy;

                      typename Properties::TypeProperties::WrapperType value;
                      pType->atlocation(x, y, value);

                      if(value.IsDefined())
                      {
                        pResult->MergeAdd(typename Properties::TypeProperties::
                                          UnitType(interval, value, value));
                      }
                    }

                    else
                    {
                      assert(currentStartTime == currentEndTime);
                    }
                  }
                }
              }
            }

            pResult->EndBulkLoad();
          }

          nRetVal = 0;
        }
      }
    }
  }

  return nRetVal;
}

/*
Template method composeFunctionmt implements the compose operator functionality
for mt datatypes.

author: Dirk Zacher
parameters: pArguments - a pointer to the arguments of compose operator
            rResult - reference to a Word containing the result
            message - message to distinguish call modes of composeFunctionmt
            rLocal - reference to a Word to store local method information
            supplier - an Address to a supplier of information of operator tree
return value: 0 if composeFunctionmt successfully executed, otherwise FAILURE
exceptions: -

*/

template <typename Type, typename Properties>
int composeFunctionmt(Word* pArguments,
                      Word& rResult,
                      int message,
                      Word& rLocal,
                      Supplier supplier)
{
  int nRetVal = FAILURE;

  if(qp != 0 &&
     pArguments != 0)
  {
    MPoint* pMPoint = static_cast<MPoint*>(pArguments[0].addr);
    Type* pType = static_cast<Type*>(pArguments[1].addr);

    if(pMPoint != 0 &&
       pType != 0)
    {
      rResult = qp->ResultStorage(supplier);

      if(rResult.addr != 0)
      {
        typename Properties::TypeProperties::MType* pResult =
        static_cast<typename Properties::TypeProperties::MType*>(rResult.addr);

        if(pResult != 0)
        {
          pResult->SetDefined(false);

          if(pMPoint->IsDefined() &&
             pType->IsDefined())
          {
            pResult->SetDefined(true);
            pResult->StartBulkLoad();

            int components = pMPoint->GetNoComponents();

            for(int i = 0; i < components; i++)
            {
              UPoint unit(0);
              pMPoint->Get(i, unit);

              double xStart = unit.p0.GetX();
              double yStart = unit.p0.GetY();
              double xEnd = unit.p1.GetX();
              double yEnd = unit.p1.GetY();
              double tStart = unit.timeInterval.start.ToDouble();
              double tEnd = unit.timeInterval.end.ToDouble();

              if(pType->IsValidLocation(xStart, yStart, tStart) &&
                 pType->IsValidLocation(yEnd, yEnd, tEnd))
              {
                Index<3> startIndex = pType->GetLocationIndex(xStart, yStart,
                                                              tStart);
                Index<3> endIndex = pType->GetLocationIndex(xEnd, yEnd,
                                                            tEnd);

                if(startIndex == endIndex)
                {
                  typename Properties::TypeProperties::WrapperType value;
                  pType->atlocation(xStart, yStart, tStart, value);

                  if(value.IsDefined())
                  {
                    pResult->MergeAdd(typename Properties::TypeProperties::
                                      UnitType(unit.timeInterval,
                                               value, value));
                  }
                }

                else
                {
                  DateTime startTime = unit.timeInterval.start;
                  DateTime endTime = unit.timeInterval.end;
                  DateTime duration = endTime - startTime;

                  mtCellIterator<Type> cellIterator(*pType,
                                                    xStart, yStart,
                                                    xEnd, yEnd,
                                                    startTime.ToDouble(),
                                                    endTime.ToDouble());
                  double dx = xEnd - xStart;
                  double dy = yEnd - yStart;

                  while(cellIterator.HasNext())
                  {
                    pair<double,double> p = cellIterator.Next();
                    DateTime currentStartTime = startTime +
                                                (duration * p.first);
                    DateTime currentEndTime = startTime +
                                              (duration * p.second);

                    if(currentEndTime > currentStartTime)
                    {
                      Interval<Instant> interval(currentStartTime,
                                                 currentEndTime,
                                                 true, false);
                      double delta  =(p.first + p.second) / 2.0;
                      double x = xStart + delta * dx;
                      double y = yStart + delta * dy;
                      double t = tStart + delta * duration.ToDouble();

                      typename Properties::TypeProperties::WrapperType value;
                      pType->atlocation(x, y, t, value);

                      if(value.IsDefined())
                      {
                        pResult->MergeAdd(typename Properties::TypeProperties::
                                          UnitType(interval, value, value));
                      }
                    }

                    else
                    {
                      assert(currentStartTime == currentEndTime);
                    }
                  }
                }
              }
            }

            pResult->EndBulkLoad();
          }

          nRetVal = 0;
        }
      }
    }
  }

  return nRetVal;
}

/*
definition of composeFunctions array.

*/

ValueMapping composeFunctions[] =
{
  composeFunctiont<tint, tProperties<int> >,
  composeFunctiont<treal, tProperties<double> >,
  composeFunctiont<tbool, tProperties<char> >,
  composeFunctiont<tstring, tProperties<std::string> >,
  composeFunctionmt<mtint, mtProperties<int> >,
  composeFunctionmt<mtreal, mtProperties<double> >,
  composeFunctionmt<mtbool, mtProperties<char> >,
  composeFunctionmt<mtstring, mtProperties<std::string> >,
  0
};

/*
Method composeSelectFunction returns the index of specific compose function
in composeFunctions array depending on the arguments.

author: Dirk Zacher
parameters: arguments - arguments of compose operator
return value: index of specific compose function in composeFunctions
exceptions: -

*/

int composeSelectFunction(ListExpr arguments)
{
  int functionIndex = -1;

  if(arguments != 0)
  {
    NList argumentsList(arguments);
    NList argument1 = argumentsList.first();
    NList argument2 = argumentsList.second();

    if(argument1.isSymbol(MPoint::BasicType()))
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
        if(argument2.isSymbol(TYPE_NAMES_ARRAY[i]))
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
Method composeTypeMappingFunction returns the return value type
of compose operator in the form of a ListExpr.

author: Dirk Zacher
parameters: arguments - arguments of compose operator
return value: return value type of compose operator
exceptions: -

*/

ListExpr composeTypeMappingFunction(ListExpr arguments)
{
  ListExpr type = NList::typeError("Operator compose expects "
                                   "a mpoint and a t type or a mt type.");

  NList argumentsList(arguments);

  if(argumentsList.hasLength(2))
  {
    std::string argument1 = argumentsList.first().str();
    std::string argument2 = argumentsList.second().str();

    if(argument1 == MPoint::BasicType() &&
      (IstType(argument2) ||
       IsmtType(argument2)))
    {
      type = NList(GetMType(argument2)).listExpr();
    }
  }

  return type;
}

}
