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

#include "map2.h"
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
Template method map2Functiontt implements the map2 operator functionality
between a t datatype and a t datatype.

author: Dirk Zacher
parameters: pArguments - a pointer to the arguments of map2 operator
            rResult - reference to a Word containing the result
            message - message to distinguish call modes of map2Functiontt
            rLocal - reference to a Word to store local method information
            supplier - an Address to a supplier of information of operator tree
return value: 0 if map2Functiontt successfully executed, otherwise FAILURE
exceptions: -

*/

template <typename SourceType1, typename SourceType1Properties,
          typename SourceType2, typename SourceType2Properties,
          typename DestinationType, typename DestinationTypeProperties>
int map2Functiontt(Word* pArguments,
                   Word& rResult,
                   int message,
                   Word& rLocal,
                   Supplier supplier)
{
  int nRetVal = FAILURE;

  if(qp != 0 &&
     pArguments != 0)
  {
    SourceType1* pSourceType1 = static_cast<SourceType1*>(pArguments[0].addr);
    SourceType2* pSourceType2 = static_cast<SourceType2*>(pArguments[1].addr);
    Address pFunction = pArguments[2].addr;

    if(pSourceType1 != 0 &&
       pSourceType2 != 0 &&
       pFunction != 0)
    {
      rResult = qp->ResultStorage(supplier);

      if(rResult.addr != 0)
      {
        DestinationType* pResult = static_cast<DestinationType*>(rResult.addr);

        if(pResult != 0)
        {
          pResult->SetDefined(false);

          if(pSourceType1->IsDefined() &&
             pSourceType2->IsDefined())
          {
            typename SourceType1Properties::GridType grid1;
            pSourceType1->getgrid(grid1);
            typename SourceType2Properties::GridType grid2;
            pSourceType2->getgrid(grid2);

            if(grid1.IsMatchingGrid(grid2))
            {
              double dx = (grid1.GetX() - grid2.GetX()) / grid1.GetLength();
              double dy = (grid1.GetY() - grid2.GetY()) / grid1.GetLength();
              int dc = dx + 0.5;
              int dr = dy + 0.5;
              assert(grid1.GetLength() == grid2.GetLength());
              assert(AlmostEqual(dx, dc) &&
                     AlmostEqual(dy, dr));

              typename SourceType1Properties::RectangleType boundingBox1;
              pSourceType1->bbox(boundingBox1);

              typename SourceType2Properties::RectangleType boundingBox2;
              pSourceType2->bbox(boundingBox2);

              typename SourceType1Properties::RectangleType boundingBox =
              boundingBox1.Union(boundingBox2);

              if(boundingBox.IsDefined())
              {
                pResult->SetDefined(true);
                pResult->SetGrid(grid1);

                Index<2> startIndex = pSourceType1->GetLocationIndex
                                      (boundingBox.MinD(0),
                                       boundingBox.MinD(1));

                Index<2> endIndex = pSourceType1->GetLocationIndex
                                    (boundingBox.MaxD(0),
                                     boundingBox.MaxD(1));

                ArgVector& argumentsVector = *qp->Argument(pFunction);
                Word word;

                for(int row = startIndex[1]; row <= endIndex[1]; row++)
                {
                  for(int column = startIndex[0]; column <= endIndex[0];
                      column++)
                  {
                    Index<2> index1 = (int[]){column, row};
                    typename SourceType1Properties::TypeProperties::
                    PropertiesType value1 = pSourceType1->GetValue(index1);

                    Index<2> index2 = (int[]){column + dc, row + dr};
                    typename SourceType2Properties::TypeProperties::
                    PropertiesType value2 = pSourceType2->GetValue(index2);

                    if(SourceType1Properties::TypeProperties::
                       IsUndefinedValue(value1) == false &&
                       SourceType2Properties::TypeProperties::
                       IsUndefinedValue(value2) == false)
                    {
                      typename SourceType1Properties::TypeProperties::
                      WrapperType wrappedValue1 = SourceType1Properties::
                      TypeProperties::GetWrappedValue(value1);
                      typename SourceType2Properties::TypeProperties::
                      WrapperType wrappedValue2 = SourceType2Properties::
                      TypeProperties::GetWrappedValue(value2);

                      argumentsVector[0].setAddr(&wrappedValue1);
                      argumentsVector[1].setAddr(&wrappedValue2);
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
                            pResult->SetValue(index1, unwrappedValue, true);
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
Template method map2Functiontmt implements the map2 operator functionality
between a t datatype and a mt datatype.

author: Dirk Zacher
parameters: pArguments - a pointer to the arguments of map2 operator
            rResult - reference to a Word containing the result
            message - message to distinguish call modes of map2Functiontmt
            rLocal - reference to a Word to store local method information
            supplier - an Address to a supplier of information of operator tree
return value: 0 if map2Functiontmt successfully executed, otherwise FAILURE
exceptions: -

*/

template <typename SourceType1, typename SourceType1Properties,
          typename SourceType2, typename SourceType2Properties,
          typename DestinationType, typename DestinationTypeProperties>
int map2Functiontmt(Word* pArguments,
                    Word& rResult,
                    int message,
                    Word& rLocal,
                    Supplier supplier)
{
  int nRetVal = FAILURE;

  if(qp != 0 &&
     pArguments != 0)
  {
    SourceType1* pSourceType1 = static_cast<SourceType1*>(pArguments[0].addr);
    SourceType2* pSourceType2 = static_cast<SourceType2*>(pArguments[1].addr);
    Address pFunction = pArguments[2].addr;

    if(pSourceType1 != 0 &&
       pSourceType2 != 0 &&
       pFunction != 0)
    {
      rResult = qp->ResultStorage(supplier);

      if(rResult.addr != 0)
      {
        DestinationType* pResult = static_cast<DestinationType*>(rResult.addr);

        if(pResult != 0)
        {
          pResult->SetDefined(false);

          if(pSourceType1->IsDefined() &&
             pSourceType2->IsDefined())
          {
            typename SourceType1Properties::GridType grid1;
            pSourceType1->getgrid(grid1);
            typename SourceType2Properties::GridType grid2;
            pSourceType2->getgrid(grid2);

            if(grid2.IsMatchingGrid(grid1))
            {
              double dx = (grid1.GetX() - grid2.GetX()) / grid1.GetLength();
              double dy = (grid1.GetY() - grid2.GetY()) / grid1.GetLength();
              int dc = dx + 0.5;
              int dr = dy + 0.5;
              assert(grid1.GetLength() == grid2.GetLength());
              assert(AlmostEqual(dx, dc) &&
                     AlmostEqual(dy, dr));

              typename SourceType1Properties::RectangleType boundingBox1;
              pSourceType1->bbox(boundingBox1);

              typename SourceType2Properties::RectangleType boundingBox2;
              pSourceType2->bbox(boundingBox2);

              typename SourceType1Properties::RectangleType boundingBox22
              (true, boundingBox2.MinD(0), boundingBox2.MaxD(0),
               boundingBox2.MinD(1), boundingBox2.MaxD(1));

              typename SourceType1Properties::RectangleType boundingBox =
              boundingBox1.Union(boundingBox22);

              if(boundingBox.IsDefined())
              {
                pResult->SetDefined(true);
                pResult->SetGrid(grid2);

                Index<3> startIndex = pSourceType2->GetLocationIndex
                                      (boundingBox.MinD(0),
                                       boundingBox.MinD(1),
                                       boundingBox2.MinD(2));

                Index<3> endIndex = pSourceType2->GetLocationIndex
                                    (boundingBox.MaxD(0),
                                     boundingBox.MaxD(1),
                                     boundingBox2.MaxD(2));

                ArgVector& argumentsVector = *qp->Argument(pFunction);
                Word word;

                for(int time = startIndex[2]; time <= endIndex[2]; time++)
                {
                  for(int row = startIndex[1]; row <= endIndex[1]; row++)
                  {
                    for(int column = startIndex[0]; column <= endIndex[0];
                        column++)
                    {
                      Index<2> index1 = (int[]){column + dc, row + dr};
                      typename SourceType1Properties::TypeProperties::
                      PropertiesType value1 = pSourceType1->GetValue(index1);

                      Index<3> index2 = (int[]){column, row, time};
                      typename SourceType2Properties::TypeProperties::
                      PropertiesType value2 = pSourceType2->GetValue(index2);

                      if(SourceType1Properties::TypeProperties::
                         IsUndefinedValue(value1) == false &&
                         SourceType2Properties::TypeProperties::
                         IsUndefinedValue(value2) == false)
                      {
                        typename SourceType1Properties::TypeProperties::
                        WrapperType wrappedValue1 = SourceType1Properties::
                        TypeProperties::GetWrappedValue(value1);
                        typename SourceType2Properties::TypeProperties::
                        WrapperType wrappedValue2 = SourceType2Properties::
                        TypeProperties::GetWrappedValue(value2);

                        argumentsVector[0].setAddr(&wrappedValue1);
                        argumentsVector[1].setAddr(&wrappedValue2);
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
                              pResult->SetValue(index2, unwrappedValue, true);
                            }
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
Template method map2Functionmtt implements the map2 operator functionality
between a mt datatype and a t datatype.

author: Dirk Zacher
parameters: pArguments - a pointer to the arguments of map2 operator
            rResult - reference to a Word containing the result
            message - message to distinguish call modes of map2Functionmtt
            rLocal - reference to a Word to store local method information
            supplier - an Address to a supplier of information of operator tree
return value: 0 if map2Functionmtt successfully executed, otherwise FAILURE
exceptions: -

*/

template <typename SourceType1, typename SourceType1Properties,
          typename SourceType2, typename SourceType2Properties,
          typename DestinationType, typename DestinationTypeProperties>
int map2Functionmtt(Word* pArguments,
                    Word& rResult,
                    int message,
                    Word& rLocal,
                    Supplier supplier)
{
  int nRetVal = FAILURE;

  if(qp != 0 &&
     pArguments != 0)
  {
    SourceType1* pSourceType1 = static_cast<SourceType1*>(pArguments[0].addr);
    SourceType2* pSourceType2 = static_cast<SourceType2*>(pArguments[1].addr);
    Address pFunction = pArguments[2].addr;

    if(pSourceType1 != 0 &&
       pSourceType2 != 0 &&
       pFunction != 0)
    {
      rResult = qp->ResultStorage(supplier);

      if(rResult.addr != 0)
      {
        DestinationType* pResult = static_cast<DestinationType*>(rResult.addr);

        if(pResult != 0)
        {
          pResult->SetDefined(false);

          if(pSourceType1->IsDefined() &&
             pSourceType2->IsDefined())
          {
            typename SourceType1Properties::GridType grid1;
            pSourceType1->getgrid(grid1);
            typename SourceType2Properties::GridType grid2;
            pSourceType2->getgrid(grid2);

            if(grid1.IsMatchingGrid(grid2))
            {
              double dx = (grid1.GetX() - grid2.GetX()) / grid1.GetLength();
              double dy = (grid1.GetY() - grid2.GetY()) / grid1.GetLength();
              int dc = dx + 0.5;
              int dr = dy + 0.5;
              assert(grid1.GetLength() == grid2.GetLength());
              assert(AlmostEqual(dx, dc) &&
                     AlmostEqual(dy, dr));

              typename SourceType1Properties::RectangleType boundingBox1;
              pSourceType1->bbox(boundingBox1);

              typename SourceType2Properties::RectangleType boundingBox2;
              pSourceType2->bbox(boundingBox2);

              typename SourceType2Properties::RectangleType boundingBox12
              (true, boundingBox1.MinD(0), boundingBox1.MaxD(0),
               boundingBox1.MinD(1), boundingBox1.MaxD(1));

              typename SourceType2Properties::RectangleType boundingBox =
              boundingBox2.Union(boundingBox12);

              if(boundingBox.IsDefined())
              {
                pResult->SetDefined(true);
                pResult->SetGrid(grid1);

                Index<3> startIndex = pSourceType1->GetLocationIndex
                                      (boundingBox.MinD(0),
                                       boundingBox.MinD(1),
                                       boundingBox1.MinD(2));

                Index<3> endIndex = pSourceType1->GetLocationIndex
                                    (boundingBox.MaxD(0),
                                     boundingBox.MaxD(1),
                                     boundingBox1.MaxD(2));

                ArgVector& argumentsVector = *qp->Argument(pFunction);
                Word word;

                for(int time = startIndex[2]; time <= endIndex[2]; time++)
                {
                  for(int row = startIndex[1]; row <= endIndex[1]; row++)
                  {
                    for(int column = startIndex[0]; column <= endIndex[0];
                        column++)
                    {
                      Index<3> index1 = (int[]){column, row, time};
                      typename SourceType1Properties::TypeProperties::
                      PropertiesType value1 = pSourceType1->GetValue(index1);

                      Index<2> index2 = (int[]){column + dc, row + dr};
                      typename SourceType2Properties::TypeProperties::
                      PropertiesType value2 = pSourceType2->GetValue(index2);

                      if(SourceType1Properties::TypeProperties::
                         IsUndefinedValue(value1) == false &&
                         SourceType2Properties::TypeProperties::
                         IsUndefinedValue(value2) == false)
                      {
                        typename SourceType1Properties::TypeProperties::
                        WrapperType wrappedValue1 = SourceType1Properties::
                        TypeProperties::GetWrappedValue(value1);
                        typename SourceType2Properties::TypeProperties::
                        WrapperType wrappedValue2 = SourceType2Properties::
                        TypeProperties::GetWrappedValue(value2);

                        argumentsVector[0].setAddr(&wrappedValue1);
                        argumentsVector[1].setAddr(&wrappedValue2);
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
                              pResult->SetValue(index1, unwrappedValue, true);
                            }
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
Template method map2Functionmtmt implements the map2 operator functionality
between a mt datatype and a mt datatype.

author: Dirk Zacher
parameters: pArguments - a pointer to the arguments of map2 operator
            rResult - reference to a Word containing the result
            message - message to distinguish call modes of map2Functionmtmt
            rLocal - reference to a Word to store local method information
            supplier - an Address to a supplier of information of operator tree
return value: 0 if map2Functionmtmt successfully executed, otherwise FAILURE
exceptions: -

*/

template <typename SourceType1, typename SourceType1Properties,
          typename SourceType2, typename SourceType2Properties,
          typename DestinationType, typename DestinationTypeProperties>
int map2Functionmtmt(Word* pArguments,
                     Word& rResult,
                     int message,
                     Word& rLocal,
                     Supplier supplier)
{
  int nRetVal = FAILURE;

  if(qp != 0 &&
     pArguments != 0)
  {
    SourceType1* pSourceType1 = static_cast<SourceType1*>(pArguments[0].addr);
    SourceType2* pSourceType2 = static_cast<SourceType2*>(pArguments[1].addr);
    Address pFunction = pArguments[2].addr;

    if(pSourceType1 != 0 &&
       pSourceType2 != 0 &&
       pFunction != 0)
    {
      rResult = qp->ResultStorage(supplier);

      if(rResult.addr != 0)
      {
        DestinationType* pResult = static_cast<DestinationType*>(rResult.addr);

        if(pResult != 0)
        {
          pResult->SetDefined(false);

          if(pSourceType1->IsDefined() &&
             pSourceType2->IsDefined())
          {
            typename SourceType1Properties::GridType grid1;
            pSourceType1->getgrid(grid1);
            typename SourceType2Properties::GridType grid2;
            pSourceType2->getgrid(grid2);

            if(grid1.IsMatchingGrid(grid2))
            {
              double dx = (grid1.GetX() - grid2.GetX()) / grid1.GetLength();
              double dy = (grid1.GetY() - grid2.GetY()) / grid1.GetLength();
              int dc = dx + 0.5;
              int dr = dy + 0.5;
              assert(grid1.GetLength() == grid2.GetLength());
              assert(AlmostEqual(dx, dc) &&
                     AlmostEqual(dy, dr));

              typename SourceType1Properties::RectangleType boundingBox1;
              pSourceType1->bbox(boundingBox1);

              typename SourceType2Properties::RectangleType boundingBox2;
              pSourceType2->bbox(boundingBox2);

              typename SourceType1Properties::RectangleType boundingBox =
              boundingBox1.Union(boundingBox2);

              if(boundingBox.IsDefined())
              {
                pResult->SetDefined(true);
                pResult->SetGrid(grid1);

                Index<3> startIndex = pSourceType1->GetLocationIndex
                                      (boundingBox.MinD(0),
                                       boundingBox.MinD(1),
                                       boundingBox.MinD(2));

                Index<3> endIndex = pSourceType2->GetLocationIndex
                                    (boundingBox.MaxD(0),
                                     boundingBox.MaxD(1),
                                     boundingBox.MaxD(2));

                ArgVector& argumentsVector = *qp->Argument(pFunction);
                Word word;

                for(int time = startIndex[2]; time <= endIndex[2]; time++)
                {
                  for(int row = startIndex[1]; row <= endIndex[1]; row++)
                  {
                    for(int column = startIndex[0]; column <= endIndex[0];
                        column++)
                    {
                      Index<3> index1 = (int[]){column, row, time};
                      typename SourceType1Properties::TypeProperties::
                      PropertiesType value1 = pSourceType1->GetValue(index1);

                      Index<3> index2 = (int[]){column + dc, row + dr, time};
                      typename SourceType2Properties::TypeProperties::
                      PropertiesType value2 = pSourceType2->GetValue(index2);

                      if(SourceType1Properties::TypeProperties::
                         IsUndefinedValue(value1) == false &&
                         SourceType2Properties::TypeProperties::
                         IsUndefinedValue(value2) == false)
                      {
                        typename SourceType1Properties::TypeProperties::
                        WrapperType wrappedValue1 = SourceType1Properties::
                        TypeProperties::GetWrappedValue(value1);
                        typename SourceType2Properties::TypeProperties::
                        WrapperType wrappedValue2 = SourceType2Properties::
                        TypeProperties::GetWrappedValue(value2);

                        argumentsVector[0].setAddr(&wrappedValue1);
                        argumentsVector[1].setAddr(&wrappedValue2);
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
                              pResult->SetValue(index1, unwrappedValue, true);
                            }
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
definition of map2Functions array.

*/

ValueMapping map2Functions[] =
{
  map2Functiontt<tint, tProperties<int>,
                 tint, tProperties<int>,
                 tint, tProperties<int> >,
  map2Functiontt<tint, tProperties<int>,
                 tint, tProperties<int>,
                 treal, tProperties<double> >,
  map2Functiontt<tint, tProperties<int>,
                 tint, tProperties<int>,
                 tbool, tProperties<char> >,
  map2Functiontt<tint, tProperties<int>,
                 tint, tProperties<int>,
                 tstring, tProperties<std::string> >,
  map2Functiontt<tint, tProperties<int>,
                 treal, tProperties<double>,
                 tint, tProperties<int> >,
  map2Functiontt<tint, tProperties<int>,
                 treal, tProperties<double>,
                 treal, tProperties<double> >,
  map2Functiontt<tint, tProperties<int>,
                 treal, tProperties<double>,
                 tbool, tProperties<char> >,
  map2Functiontt<tint, tProperties<int>,
                 treal, tProperties<double>,
                 tstring, tProperties<std::string> >,
  map2Functiontt<tint, tProperties<int>,
                 tbool, tProperties<char>,
                 tint, tProperties<int> >,
  map2Functiontt<tint, tProperties<int>,
                 tbool, tProperties<char>,
                 treal, tProperties<double> > ,
  map2Functiontt<tint, tProperties<int>,
                 tbool, tProperties<char>,
                 tbool, tProperties<char> >,
  map2Functiontt<tint, tProperties<int>,
                 tbool, tProperties<char>,
                 tstring, tProperties<std::string> >,
  map2Functiontt<tint, tProperties<int>,
                 tstring, tProperties<std::string>,
                 tint, tProperties<int> >,
  map2Functiontt<tint, tProperties<int>,
                 tstring, tProperties<std::string>,
                 treal, tProperties<double> >,
  map2Functiontt<tint, tProperties<int>,
                 tstring, tProperties<std::string>,
                 tbool, tProperties<char> >,
  map2Functiontt<tint, tProperties<int>,
                 tstring, tProperties<std::string>,
                 tstring, tProperties<std::string> >,
  map2Functiontmt<tint, tProperties<int>,
                  mtint, mtProperties<int>,
                  mtint, mtProperties<int> >,
  map2Functiontmt<tint, tProperties<int>,
                  mtint, mtProperties<int>,
                  mtreal, mtProperties<double> >,
  map2Functiontmt<tint, tProperties<int>,
                  mtint, mtProperties<int>,
                  mtbool, tProperties<char> >,
  map2Functiontmt<tint, tProperties<int>,
                  mtint, mtProperties<int>,
                  mtstring, mtProperties<std::string> >,
  map2Functiontmt<tint, tProperties<int>,
                  mtreal, mtProperties<double>,
                  mtint, mtProperties<int> >,
  map2Functiontmt<tint, tProperties<int>,
                  mtreal, mtProperties<double>,
                  mtreal, tProperties<double> >,
  map2Functiontmt<tint, tProperties<int>,
                  mtreal, mtProperties<double>,
                  mtbool, mtProperties<char> >,
  map2Functiontmt<tint, tProperties<int>,
                  mtreal, mtProperties<double>,
                  mtstring, mtProperties<std::string> >,
  map2Functiontmt<tint, tProperties<int>,
                  mtbool, mtProperties<char>,
                  mtint, mtProperties<int> >,
  map2Functiontmt<tint, tProperties<int>,
                  mtbool, mtProperties<char>,
                  mtreal, mtProperties<double> >,
  map2Functiontmt<tint, tProperties<int>,
                  mtbool, mtProperties<char>,
                  mtbool, mtProperties<char> >,
  map2Functiontmt<tint, tProperties<int>,
                  mtbool, mtProperties<char>,
                  mtstring, mtProperties<std::string> >,
  map2Functiontmt<tint, tProperties<int>,
                  mtstring, mtProperties<std::string>,
                  mtint, mtProperties<int> >,
  map2Functiontmt<tint, tProperties<int>,
                  mtstring, mtProperties<std::string>,
                  mtreal, mtProperties<double> >,
  map2Functiontmt<tint, tProperties<int>,
                  mtstring, mtProperties<std::string>,
                  mtbool, mtProperties<char> >,
  map2Functiontmt<tint, tProperties<int>,
                  mtstring, mtProperties<std::string>,
                  mtstring, mtProperties<std::string> >,

  map2Functiontt<treal, tProperties<double>,
                 tint, tProperties<int>,
                 tint, tProperties<int> >,
  map2Functiontt<treal, tProperties<double>,
                 tint, tProperties<int>,
                 treal, tProperties<double> >,
  map2Functiontt<treal, tProperties<double>,
                 tint, tProperties<int>,
                 tbool, tProperties<char> >,
  map2Functiontt<treal, tProperties<double>,
                 tint, tProperties<int>,
                 tstring, tProperties<std::string> >,
  map2Functiontt<treal, tProperties<double>,
                 treal, tProperties<double>,
                 tint, tProperties<int> >,
  map2Functiontt<treal, tProperties<double>,
                 treal, tProperties<double>,
                 treal, tProperties<double> >,
  map2Functiontt<treal, tProperties<double>,
                 treal, tProperties<double>,
                 tbool, tProperties<char> >,
  map2Functiontt<treal, tProperties<double>,
                 treal, tProperties<double>,
                 tstring, tProperties<std::string> >,
  map2Functiontt<treal, tProperties<double>,
                 tbool, tProperties<char>,
                 tint, tProperties<int> >,
  map2Functiontt<treal, tProperties<double>,
                 tbool, tProperties<char>,
                 treal, tProperties<double> >,
  map2Functiontt<treal, tProperties<double>,
                 tbool, tProperties<char>,
                 tbool, tProperties<char> >,
  map2Functiontt<treal, tProperties<double>,
                 tbool, tProperties<char>,
                 tstring, tProperties<std::string> >,
  map2Functiontt<treal, tProperties<double>,
                 tstring, tProperties<std::string>,
                 tint, tProperties<int> >,
  map2Functiontt<treal, tProperties<double>,
                 tstring, tProperties<std::string>,
                 treal, tProperties<double> >,
  map2Functiontt<treal, tProperties<double>,
                 tstring, tProperties<std::string>,
                 tbool, tProperties<char> >,
  map2Functiontt<treal, tProperties<double>,
                 tstring, tProperties<std::string>,
                 tstring, tProperties<std::string> >,
  map2Functiontmt<treal, tProperties<double>,
                  mtint, mtProperties<int>,
                  mtint, mtProperties<int> >,
  map2Functiontmt<treal, tProperties<double>,
                  mtint, mtProperties<int>,
                  mtreal, mtProperties<double> >,
  map2Functiontmt<treal, tProperties<double>,
                  mtint, mtProperties<int>,
                  mtbool, mtProperties<char> >,
  map2Functiontmt<treal, tProperties<double>,
                  mtint, mtProperties<int>,
                  mtstring, mtProperties<std::string> >,
  map2Functiontmt<treal, tProperties<double>,
                  mtreal, mtProperties<double>,
                  mtint, mtProperties<int> >,
  map2Functiontmt<treal, tProperties<double>,
                  mtreal, mtProperties<double>,
                  mtreal, mtProperties<double> >,
  map2Functiontmt<treal, tProperties<double>,
                  mtreal, mtProperties<double>,
                  mtbool, mtProperties<char> >,
  map2Functiontmt<treal, tProperties<double>,
                  mtreal, mtProperties<double>,
                  mtstring, mtProperties<std::string> >,
  map2Functiontmt<treal, tProperties<double>,
                  mtbool, mtProperties<char>,
                  mtint, mtProperties<int> >,
  map2Functiontmt<treal, tProperties<double>,
                  mtbool, mtProperties<char>,
                  mtreal, mtProperties<double> >,
  map2Functiontmt<treal, tProperties<double>,
                  mtbool, mtProperties<char>,
                  mtbool, mtProperties<char> >,
  map2Functiontmt<treal, tProperties<double>,
                  mtbool, mtProperties<char>,
                  mtstring, mtProperties<std::string> >,
  map2Functiontmt<treal, tProperties<double>,
                  mtstring, mtProperties<std::string>,
                  mtint, mtProperties<int> >,
  map2Functiontmt<treal, tProperties<double>,
                  mtstring, mtProperties<std::string>,
                  mtreal, mtProperties<double> >,
  map2Functiontmt<treal, tProperties<double>,
                  mtstring, mtProperties<std::string>,
                  mtbool, mtProperties<char> >,
  map2Functiontmt<treal, tProperties<double>,
                  mtstring, mtProperties<std::string>,
                  mtstring, mtProperties<std::string> >,

  map2Functiontt<tbool, tProperties<char>,
                 tint, tProperties<int>,
                 tint, tProperties<int> >,
  map2Functiontt<tbool, tProperties<char>,
                 tint, tProperties<int>,
                 treal, tProperties<double> >,
  map2Functiontt<tbool, tProperties<char>,
                 tint, tProperties<int>,
                 tbool, tProperties<char> >,
  map2Functiontt<tbool, tProperties<char>,
                 tint, tProperties<int>,
                 tstring, tProperties<std::string> >,
  map2Functiontt<tbool, tProperties<char>,
                 treal, tProperties<double>,
                 tint, tProperties<int> >,
  map2Functiontt<tbool, tProperties<char>,
                 treal, tProperties<double>,
                 treal, tProperties<double> >,
  map2Functiontt<tbool, tProperties<char>,
                 treal, tProperties<double>,
                 tbool, tProperties<char> >,
  map2Functiontt<tbool, tProperties<char>,
                 treal, tProperties<double>,
                 tstring, tProperties<std::string> >,
  map2Functiontt<tbool, tProperties<char>,
                 tbool, tProperties<char>,
                 tint, tProperties<int> >,
  map2Functiontt<tbool, tProperties<char>,
                 tbool, tProperties<char>,
                 treal, tProperties<double> >,
  map2Functiontt<tbool, tProperties<char>,
                 tbool, tProperties<char>,
                 tbool, tProperties<char> >,
  map2Functiontt<tbool, tProperties<char>,
                 tbool, tProperties<char>,
                 tstring, tProperties<std::string> >,
  map2Functiontt<tbool, tProperties<char>,
                 tstring, tProperties<std::string>,
                 tint, tProperties<int> >,
  map2Functiontt<tbool, tProperties<char>,
                 tstring, tProperties<std::string>,
                 treal, tProperties<double> >,
  map2Functiontt<tbool, tProperties<char>,
                 tstring, tProperties<std::string>,
                 tbool, tProperties<char> >,
  map2Functiontt<tbool, tProperties<char>,
                 tstring, tProperties<std::string>,
                 tstring, tProperties<std::string> >,
  map2Functiontmt<tbool, tProperties<char>,
                  mtint, mtProperties<int>,
                  mtint, mtProperties<int> >,
  map2Functiontmt<tbool, tProperties<char>,
                  mtint, mtProperties<int>,
                  mtreal, mtProperties<double> >,
  map2Functiontmt<tbool, tProperties<char>,
                  mtint, mtProperties<int>,
                  mtbool, mtProperties<char> >,
  map2Functiontmt<tbool, tProperties<char>,
                  mtint, mtProperties<int>,
                  mtstring, mtProperties<std::string> >,
  map2Functiontmt<tbool, tProperties<char>,
                  mtreal, mtProperties<double>,
                  mtint, mtProperties<int> >,
  map2Functiontmt<tbool, tProperties<char>,
                  mtreal, mtProperties<double>,
                  mtreal, mtProperties<double> >,
  map2Functiontmt<tbool, tProperties<char>,
                  mtreal, mtProperties<double>,
                  mtbool, mtProperties<char> >,
  map2Functiontmt<tbool, tProperties<char>,
                  mtreal, mtProperties<double>,
                  mtstring, mtProperties<std::string> >,
  map2Functiontmt<tbool, tProperties<char>,
                  mtbool, mtProperties<char>,
                  mtint, mtProperties<int> >,
  map2Functiontmt<tbool, tProperties<char>,
                  mtbool, mtProperties<char>,
                  mtreal, mtProperties<double> >,
  map2Functiontmt<tbool, tProperties<char>,
                  mtbool, mtProperties<char>,
                  mtbool, mtProperties<char> >,
  map2Functiontmt<tbool, tProperties<char>,
                  mtbool, mtProperties<char>,
                  mtstring, mtProperties<std::string> >,
  map2Functiontmt<tbool, tProperties<char>,
                  mtstring, mtProperties<std::string>,
                  mtint, mtProperties<int> >,
  map2Functiontmt<tbool, tProperties<char>,
                  mtstring, mtProperties<std::string>,
                  mtreal, mtProperties<double> >,
  map2Functiontmt<tbool, tProperties<char>,
                  mtstring, mtProperties<std::string>,
                  mtbool, mtProperties<char> >,
  map2Functiontmt<tbool, tProperties<char>,
                  mtstring, mtProperties<std::string>,
                  mtstring, mtProperties<std::string> >,

  map2Functiontt<tstring, tProperties<std::string>,
                 tint, tProperties<int>,
                 tint, tProperties<int> >,
  map2Functiontt<tstring, tProperties<std::string>,
                 tint, tProperties<int>,
                 treal, tProperties<double> >,
  map2Functiontt<tstring, tProperties<std::string>,
                 tint, tProperties<int>,
                 tbool, tProperties<char> >,
  map2Functiontt<tstring, tProperties<std::string>,
                 tint, tProperties<int>,
                 tstring, tProperties<std::string> >,
  map2Functiontt<tstring, tProperties<std::string>,
                 treal, tProperties<double>,
                 tint, tProperties<int> >,
  map2Functiontt<tstring, tProperties<std::string>,
                 treal, tProperties<double>,
                 treal, tProperties<double> >,
  map2Functiontt<tstring, tProperties<std::string>,
                 treal, tProperties<double>,
                 tbool, tProperties<char> >,
  map2Functiontt<tstring, tProperties<std::string>,
                 treal, tProperties<double>,
                 tstring, tProperties<std::string> >,
  map2Functiontt<tstring, tProperties<std::string>,
                 tbool, tProperties<char>,
                 tint, tProperties<int> >,
  map2Functiontt<tstring, tProperties<std::string>,
                 tbool, tProperties<char>,
                 treal, tProperties<double> >,
  map2Functiontt<tstring, tProperties<std::string>,
                 tbool, tProperties<char>,
                 tbool, tProperties<char> >,
  map2Functiontt<tstring, tProperties<std::string>,
                 tbool, tProperties<char>,
                 tstring, tProperties<std::string> >,
  map2Functiontt<tstring, tProperties<std::string>,
                 tstring, tProperties<std::string>,
                 tint, tProperties<int> >,
  map2Functiontt<tstring, tProperties<std::string>,
                 tstring, tProperties<std::string>,
                 treal, tProperties<double> >,
  map2Functiontt<tstring, tProperties<std::string>,
                 tstring, tProperties<std::string>,
                 tbool, tProperties<char> >,
  map2Functiontt<tstring, tProperties<std::string>,
                 tstring, tProperties<std::string>,
                 tstring, tProperties<std::string> >,
  map2Functiontmt<tstring, tProperties<std::string>,
                  mtint, mtProperties<int>,
                  mtint, mtProperties<int> >,
  map2Functiontmt<tstring, tProperties<std::string>,
                  mtint, mtProperties<int>,
                  mtreal, mtProperties<double> >,
  map2Functiontmt<tstring, tProperties<std::string>,
                  mtint, mtProperties<int>,
                  mtbool, mtProperties<char> >,
  map2Functiontmt<tstring, tProperties<std::string>,
                  mtint, mtProperties<int>,
                  mtstring, mtProperties<std::string> >,
  map2Functiontmt<tstring, tProperties<std::string>,
                  mtreal, mtProperties<double>,
                  mtint, mtProperties<int> >,
  map2Functiontmt<tstring, tProperties<std::string>,
                  mtreal, mtProperties<double>,
                  mtreal, mtProperties<double> >,
  map2Functiontmt<tstring, tProperties<std::string>,
                  mtreal, mtProperties<double>,
                  mtbool, mtProperties<char> >,
  map2Functiontmt<tstring, tProperties<std::string>,
                  mtreal, mtProperties<double>,
                  mtstring, mtProperties<std::string> >,
  map2Functiontmt<tstring, tProperties<std::string>,
                  mtbool, mtProperties<char>,
                  mtint, mtProperties<int> >,
  map2Functiontmt<tstring, tProperties<std::string>,
                  mtbool, mtProperties<char>,
                  mtreal, mtProperties<double> >,
  map2Functiontmt<tstring, tProperties<std::string>,
                  mtbool, mtProperties<char>,
                  mtbool, mtProperties<char> >,
  map2Functiontmt<tstring, tProperties<std::string>,
                  mtbool, mtProperties<char>,
                  mtstring, tProperties<std::string> >,
  map2Functiontmt<tstring, tProperties<std::string>,
                  mtstring, mtProperties<std::string>,
                  mtint, mtProperties<int> >,
  map2Functiontmt<tstring, tProperties<std::string>,
                  mtstring, mtProperties<std::string>,
                  mtreal, mtProperties<double> >,
  map2Functiontmt<tstring, tProperties<std::string>,
                  mtstring, mtProperties<std::string>,
                  mtbool, mtProperties<char> >,
  map2Functiontmt<tstring, tProperties<std::string>,
                  mtstring, mtProperties<std::string>,
                  mtstring, mtProperties<std::string> >,

  map2Functionmtt<mtint, mtProperties<int>,
                  tint, tProperties<int>,
                  mtint, mtProperties<int> >,
  map2Functionmtt<mtint, mtProperties<int>,
                  tint, tProperties<int>,
                  mtreal, mtProperties<double> >,
  map2Functionmtt<mtint, mtProperties<int>,
                  tint, tProperties<int>,
                  mtbool, mtProperties<char> >,
  map2Functionmtt<mtint, mtProperties<int>,
                  tint, tProperties<int>,
                  mtstring, mtProperties<std::string> >,
  map2Functionmtt<mtint, mtProperties<int>,
                  treal, tProperties<double>,
                  mtint, mtProperties<int> >,
  map2Functionmtt<mtint, mtProperties<int>,
                  treal, tProperties<double>,
                  mtreal, mtProperties<double> >,
  map2Functionmtt<mtint, mtProperties<int>,
                  treal, tProperties<double>,
                  mtbool, mtProperties<char> >,
  map2Functionmtt<mtint, mtProperties<int>,
                  treal, tProperties<double>,
                  mtstring, mtProperties<std::string> >,
  map2Functionmtt<mtint, mtProperties<int>,
                  tbool, tProperties<char>,
                  mtint, mtProperties<int> >,
  map2Functionmtt<mtint, mtProperties<int>,
                  tbool, tProperties<char>,
                  mtreal, mtProperties<double> >,
  map2Functionmtt<mtint, mtProperties<int>,
                  tbool, tProperties<char>,
                  mtbool, mtProperties<char> >,
  map2Functionmtt<mtint, mtProperties<int>,
                  tbool, tProperties<char>,
                  mtstring, mtProperties<std::string> >,
  map2Functionmtt<mtint, mtProperties<int>,
                  tstring, tProperties<std::string>,
                  mtint, mtProperties<int> >,
  map2Functionmtt<mtint, mtProperties<int>,
                  tstring, tProperties<std::string>,
                  mtreal, mtProperties<double> >,
  map2Functionmtt<mtint, mtProperties<int>,
                  tstring, tProperties<std::string>,
                  mtbool, mtProperties<char> >,
  map2Functionmtt<mtint, mtProperties<int>,
                  tstring, tProperties<std::string>,
                  mtstring, mtProperties<std::string> >,
  map2Functionmtmt<mtint, mtProperties<int>,
                   mtint, mtProperties<int>,
                   mtint, mtProperties<int> >,
  map2Functionmtmt<mtint, mtProperties<int>,
                   mtint, mtProperties<int>,
                   mtreal, mtProperties<double> >,
  map2Functionmtmt<mtint, mtProperties<int>,
                   mtint, mtProperties<int>,
                   mtbool, mtProperties<char> >,
  map2Functionmtmt<mtint, mtProperties<int>,
                   mtint, mtProperties<int>,
                   mtstring, mtProperties<std::string> >,
  map2Functionmtmt<mtint, mtProperties<int>,
                   mtreal, mtProperties<double>,
                   mtint, mtProperties<int> >,
  map2Functionmtmt<mtint, mtProperties<int>,
                   mtreal, mtProperties<double>,
                   mtreal, mtProperties<double> >,
  map2Functionmtmt<mtint, mtProperties<int>,
                   mtreal, mtProperties<double>,
                   mtbool, mtProperties<char> >,
  map2Functionmtmt<mtint, mtProperties<int>,
                   mtreal, mtProperties<double>,
                   mtstring, mtProperties<std::string> >,
  map2Functionmtmt<mtint, mtProperties<int>,
                   mtbool, mtProperties<char>,
                   mtint, mtProperties<int> >,
  map2Functionmtmt<mtint, mtProperties<int>,
                   mtbool, mtProperties<char>,
                   mtreal, mtProperties<double> >,
  map2Functionmtmt<mtint, mtProperties<int>,
                   mtbool, mtProperties<char>,
                   mtbool, mtProperties<char> >,
  map2Functionmtmt<mtint, mtProperties<int>,
                   mtbool, mtProperties<char>,
                   mtstring, mtProperties<std::string> >,
  map2Functionmtmt<mtint, mtProperties<int>,
                   mtstring, mtProperties<std::string>,
                   mtint, mtProperties<int> >,
  map2Functionmtmt<mtint, mtProperties<int>,
                   mtstring, mtProperties<std::string>,
                   mtreal, mtProperties<double> >,
  map2Functionmtmt<mtint, mtProperties<int>,
                   mtstring, mtProperties<std::string>,
                   mtbool, mtProperties<char> >,
  map2Functionmtmt<mtint, mtProperties<int>,
                   mtstring, mtProperties<std::string>,
                   mtstring, mtProperties<std::string> >,

  map2Functionmtt<mtreal, mtProperties<double>,
                  tint, tProperties<int>,
                  mtint, mtProperties<int> >,
  map2Functionmtt<mtreal, mtProperties<double>,
                  tint, tProperties<int>,
                  mtreal, mtProperties<double> >,
  map2Functionmtt<mtreal, mtProperties<double>,
                  tint, tProperties<int>,
                  mtbool, mtProperties<char> >,
  map2Functionmtt<mtreal, mtProperties<double>,
                  tint, tProperties<int>,
                  mtstring, mtProperties<std::string> >,
  map2Functionmtt<mtreal, mtProperties<double>,
                  treal, tProperties<double>,
                  mtint, mtProperties<int> >,
  map2Functionmtt<mtreal, mtProperties<double>,
                  treal, tProperties<double>,
                  mtreal, mtProperties<double> >,
  map2Functionmtt<mtreal, mtProperties<double>,
                  treal, tProperties<double>,
                  mtbool, mtProperties<char> >,
  map2Functionmtt<mtreal, mtProperties<double>,
                  treal, tProperties<double>,
                  mtstring, mtProperties<std::string> >,
  map2Functionmtt<mtreal, mtProperties<double>,
                  tbool, tProperties<char>,
                  mtint, mtProperties<int> >,
  map2Functionmtt<mtreal, mtProperties<double>,
                  tbool, tProperties<char>,
                  mtreal, mtProperties<double> >,
  map2Functionmtt<mtreal, mtProperties<double>,
                  tbool, tProperties<char>,
                  mtbool, mtProperties<char> >,
  map2Functionmtt<mtreal, mtProperties<double>,
                  tbool, tProperties<char>,
                  mtstring, mtProperties<std::string> >,
  map2Functionmtt<mtreal, mtProperties<double>,
                  tstring, tProperties<std::string>,
                  mtint, mtProperties<int> >,
  map2Functionmtt<mtreal, mtProperties<double>,
                  tstring, tProperties<std::string>,
                  mtreal, mtProperties<double> >,
  map2Functionmtt<mtreal, mtProperties<double>,
                  tstring, tProperties<std::string>,
                  mtbool, mtProperties<char> >,
  map2Functionmtt<mtreal, mtProperties<double>,
                  tstring, tProperties<std::string>,
                  mtstring, mtProperties<std::string> >,
  map2Functionmtmt<mtreal, mtProperties<double>,
                   mtint, mtProperties<int>,
                   mtint, mtProperties<int> >,
  map2Functionmtmt<mtreal, mtProperties<double>,
                   mtint, mtProperties<int>,
                   mtreal, mtProperties<double> >,
  map2Functionmtmt<mtreal, mtProperties<double>,
                   mtint, mtProperties<int>,
                   mtbool, mtProperties<char> >,
  map2Functionmtmt<mtreal, mtProperties<double>,
                   mtint, mtProperties<int>,
                   mtstring, mtProperties<std::string> >,
  map2Functionmtmt<mtreal, mtProperties<double>,
                   mtreal, mtProperties<double>,
                   mtint, mtProperties<int> >,
  map2Functionmtmt<mtreal, mtProperties<double>,
                   mtreal, mtProperties<double>,
                   mtreal, mtProperties<double> >,
  map2Functionmtmt<mtreal, mtProperties<double>,
                   mtreal, mtProperties<double>,
                   mtbool, mtProperties<char> >,
  map2Functionmtmt<mtreal, mtProperties<double>,
                   mtreal, mtProperties<double>,
                   mtstring, mtProperties<std::string> >,
  map2Functionmtmt<mtreal, mtProperties<double>,
                   mtbool, mtProperties<char>,
                   mtint, mtProperties<int> >,
  map2Functionmtmt<mtreal, mtProperties<double>,
                   mtbool, mtProperties<char>,
                   mtreal, mtProperties<double> >,
  map2Functionmtmt<mtreal, mtProperties<double>,
                   mtbool, mtProperties<char>,
                   mtbool, mtProperties<char> >,
  map2Functionmtmt<mtreal, mtProperties<double>,
                   mtbool, mtProperties<char>,
                   mtstring, mtProperties<std::string> >,
  map2Functionmtmt<mtreal, mtProperties<double>,
                   mtstring, mtProperties<std::string>,
                   mtint, mtProperties<int> >,
  map2Functionmtmt<mtreal, mtProperties<double>,
                   mtstring, mtProperties<std::string>,
                   mtreal, mtProperties<double> >,
  map2Functionmtmt<mtreal, mtProperties<double>,
                   mtstring, mtProperties<std::string>,
                   mtbool, mtProperties<char> >,
  map2Functionmtmt<mtreal, mtProperties<double>,
                   mtstring, mtProperties<std::string>,
                   mtstring, mtProperties<std::string> >,

  map2Functionmtt<mtbool, mtProperties<char>,
                  tint, tProperties<int>,
                  mtint, mtProperties<int> >,
  map2Functionmtt<mtbool, mtProperties<char>,
                  tint, tProperties<int>,
                  mtreal, mtProperties<double> >,
  map2Functionmtt<mtbool, mtProperties<char>,
                  tint, tProperties<int>,
                  mtbool, mtProperties<char> >,
  map2Functionmtt<mtbool, mtProperties<char>,
                  tint, tProperties<int>,
                  mtstring, mtProperties<std::string> >,
  map2Functionmtt<mtbool, mtProperties<char>,
                  treal, tProperties<double>,
                  mtint, mtProperties<int> >,
  map2Functionmtt<mtbool, mtProperties<char>,
                  treal, tProperties<double>,
                  mtreal, mtProperties<double> >,
  map2Functionmtt<mtbool, mtProperties<char>,
                  treal, tProperties<double>,
                  mtbool, mtProperties<char> >,
  map2Functionmtt<mtbool, mtProperties<char>,
                  treal, tProperties<double>,
                  mtstring, mtProperties<std::string> >,
  map2Functionmtt<mtbool, mtProperties<char>,
                  tbool, tProperties<char>,
                  mtint, mtProperties<int> >,
  map2Functionmtt<mtbool, mtProperties<char>,
                  tbool, tProperties<char>,
                  mtreal, mtProperties<double> >,
  map2Functionmtt<mtbool, mtProperties<char>,
                  tbool, tProperties<char>,
                  mtbool, mtProperties<char> >,
  map2Functionmtt<mtbool, mtProperties<char>,
                  tbool, tProperties<char>,
                  mtstring, mtProperties<std::string> >,
  map2Functionmtt<mtbool, mtProperties<char>,
                  tstring, tProperties<std::string>,
                  mtint, mtProperties<int> >,
  map2Functionmtt<mtbool, mtProperties<char>,
                  tstring, tProperties<std::string>,
                  mtreal, mtProperties<double> >,
  map2Functionmtt<mtbool, mtProperties<char>,
                  tstring, tProperties<std::string>,
                  mtbool, mtProperties<char> >,
  map2Functionmtt<mtbool, mtProperties<char>,
                  tstring, tProperties<std::string>,
                  mtstring, mtProperties<std::string> >,
  map2Functionmtmt<mtbool, mtProperties<char>,
                   mtint, mtProperties<int>,
                   mtint, mtProperties<int> >,
  map2Functionmtmt<mtbool, mtProperties<char>,
                   mtint, mtProperties<int>,
                   mtreal, mtProperties<double> >,
  map2Functionmtmt<mtbool, mtProperties<char>,
                   mtint, mtProperties<int>,
                   mtbool, mtProperties<char> >,
  map2Functionmtmt<mtbool, mtProperties<char>,
                   mtint, mtProperties<int>,
                   mtstring, mtProperties<std::string> >,
  map2Functionmtmt<mtbool, mtProperties<char>,
                   mtreal, mtProperties<double>,
                   mtint, mtProperties<int> >,
  map2Functionmtmt<mtbool, mtProperties<char>,
                   mtreal, mtProperties<double>,
                   mtreal, mtProperties<double> >,
  map2Functionmtmt<mtbool, mtProperties<char>,
                   mtreal, mtProperties<double>,
                   mtbool, mtProperties<char> >,
  map2Functionmtmt<mtbool, mtProperties<char>,
                   mtreal, mtProperties<double>,
                   mtstring, mtProperties<std::string> >,
  map2Functionmtmt<mtbool, mtProperties<char>,
                   mtbool, mtProperties<char>,
                   mtint, mtProperties<int> >,
  map2Functionmtmt<mtbool, mtProperties<char>,
                   mtbool, mtProperties<char>,
                   mtreal, mtProperties<double> >,
  map2Functionmtmt<mtbool, mtProperties<char>,
                   mtbool, mtProperties<char>,
                   mtbool, mtProperties<char> >,
  map2Functionmtmt<mtbool, mtProperties<char>,
                   mtbool, mtProperties<char>,
                   mtstring, mtProperties<std::string> >,
  map2Functionmtmt<mtbool, mtProperties<char>,
                   mtstring, mtProperties<std::string>,
                   mtint, mtProperties<int> >,
  map2Functionmtmt<mtbool, mtProperties<char>,
                   mtstring, mtProperties<std::string>,
                   mtreal, mtProperties<double> >,
  map2Functionmtmt<mtbool, mtProperties<char>,
                   mtstring, mtProperties<std::string>,
                   mtbool, mtProperties<char> >,
  map2Functionmtmt<mtbool, mtProperties<char>,
                   mtstring, mtProperties<std::string>,
                   mtstring, mtProperties<std::string> >,

  map2Functionmtt<mtstring, mtProperties<std::string>,
                  tint, tProperties<int>,
                  mtint, mtProperties<int> >,
  map2Functionmtt<mtstring, mtProperties<std::string>,
                  tint, tProperties<int>,
                  mtreal, mtProperties<double> >,
  map2Functionmtt<mtstring, mtProperties<std::string>,
                  tint, tProperties<int>,
                  mtbool, mtProperties<char> >,
  map2Functionmtt<mtstring, mtProperties<std::string>,
                  tint, tProperties<int>,
                  mtstring, mtProperties<std::string> >,
  map2Functionmtt<mtstring, mtProperties<std::string>,
                  treal, tProperties<double>,
                  mtint, mtProperties<int> >,
  map2Functionmtt<mtstring, mtProperties<std::string>,
                  treal, tProperties<double>,
                  mtreal, mtProperties<double> >,
  map2Functionmtt<mtstring, mtProperties<std::string>,
                  treal, tProperties<double>,
                  mtbool, mtProperties<char> >,
  map2Functionmtt<mtstring, mtProperties<std::string>,
                  treal, tProperties<double>,
                  mtstring, mtProperties<std::string> >,
  map2Functionmtt<mtstring, mtProperties<std::string>,
                  tbool, tProperties<char>,
                  mtint, mtProperties<int> >,
  map2Functionmtt<mtstring, mtProperties<std::string>,
                  tbool, tProperties<char>,
                  mtreal, mtProperties<double> >,
  map2Functionmtt<mtstring, mtProperties<std::string>,
                  tbool, tProperties<char>,
                  mtbool, mtProperties<char> >,
  map2Functionmtt<mtstring, mtProperties<std::string>,
                  tbool, tProperties<char>,
                  mtstring, mtProperties<std::string> >,
  map2Functionmtt<mtstring, mtProperties<std::string>,
                  tstring, tProperties<std::string>,
                  mtint, mtProperties<int> >,
  map2Functionmtt<mtstring, mtProperties<std::string>,
                  tstring, tProperties<std::string>,
                  mtreal, mtProperties<double> >,
  map2Functionmtt<mtstring, mtProperties<std::string>,
                  tstring, tProperties<std::string>,
                  mtbool, mtProperties<char> >,
  map2Functionmtt<mtstring, mtProperties<std::string>,
                  tstring, tProperties<std::string>,
                  mtstring, mtProperties<std::string> >,
  map2Functionmtmt<mtstring, mtProperties<std::string>,
                   mtint, mtProperties<int>,
                   mtint, mtProperties<int> >,
  map2Functionmtmt<mtstring, mtProperties<std::string>,
                   mtint, mtProperties<int>,
                   mtreal, mtProperties<double> >,
  map2Functionmtmt<mtstring, mtProperties<std::string>,
                   mtint, mtProperties<int>,
                   mtbool, mtProperties<char> >,
  map2Functionmtmt<mtstring, mtProperties<std::string>,
                   mtint, mtProperties<int>,
                   mtstring, mtProperties<std::string> >,
  map2Functionmtmt<mtstring, mtProperties<std::string>,
                   mtreal, mtProperties<double>,
                   mtint, mtProperties<int> >,
  map2Functionmtmt<mtstring, mtProperties<std::string>,
                   mtreal, mtProperties<double>,
                   mtreal, mtProperties<double> >,
  map2Functionmtmt<mtstring, mtProperties<std::string>,
                   mtreal, mtProperties<double>,
                   mtbool, mtProperties<char> >,
  map2Functionmtmt<mtstring, mtProperties<std::string>,
                   mtreal, mtProperties<double>,
                   mtstring, mtProperties<std::string> >,
  map2Functionmtmt<mtstring, mtProperties<std::string>,
                   mtbool, mtProperties<char>,
                   mtint, mtProperties<int> >,
  map2Functionmtmt<mtstring, mtProperties<std::string>,
                   mtbool, mtProperties<char>,
                   mtreal, mtProperties<double> >,
  map2Functionmtmt<mtstring, mtProperties<std::string>,
                   mtbool, mtProperties<char>,
                   mtbool, mtProperties<char> >,
  map2Functionmtmt<mtstring, mtProperties<std::string>,
                   mtbool, mtProperties<char>,
                   mtstring, mtProperties<std::string> >,
  map2Functionmtmt<mtstring, mtProperties<std::string>,
                   mtstring, mtProperties<std::string>,
                   mtint, mtProperties<int> >,
  map2Functionmtmt<mtstring, mtProperties<std::string>,
                   mtstring, mtProperties<std::string>,
                   mtreal, mtProperties<double> >,
  map2Functionmtmt<mtstring, mtProperties<std::string>,
                   mtstring, mtProperties<std::string>,
                   mtbool, mtProperties<char> >,
  map2Functionmtmt<mtstring, mtProperties<std::string>,
                   mtstring, mtProperties<std::string>,
                   mtstring, mtProperties<std::string> >,
  0
};

/*
Method map2SelectFunction returns the index of specific map2 function
in map2Functions array depending on the arguments.

author: Dirk Zacher
parameters: arguments - arguments of map2 operator
return value: index of specific map2 function in map2Functions
exceptions: -

*/

int map2SelectFunction(ListExpr arguments)
{
  int functionIndex = -1;

  if(arguments != 0)
  {
    NList argumentsList(arguments);

    if(argumentsList.hasLength(3))
    {
      NList argument1 = argumentsList.first();
      NList argument2 = argumentsList.second();
      std::string argument3= argumentsList.third().fourth().str();
      
      int argument1Index = -1;
      int argument2Index = -1;
      int argument3Index = -1;

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

        if(argument2.isSymbol(TYPE_NAMES_ARRAY[i]))
        {
          argument2Index = i;
        }
        
        if(argument3 == TYPE_NAMES_ARRAY[i])
        {
          argument3Index = i;
        }
      }
      
      if(argument1Index >= 0 &&
         argument2Index >= 0 &&
         argument3Index >= 0)
      {
        functionIndex = (argument1Index * 32) +
                        (argument2Index * 4) +
                        (argument3Index % 4);
      }
    }
  }

  return functionIndex;
}

/*
Method map2TypeMappingFunction returns the return value type
of map2 operator in the form of a ListExpr.

author: Dirk Zacher
parameters: arguments - arguments of map2 operator
return value: return value type of map2 operator
exceptions: -

*/

ListExpr map2TypeMappingFunction(ListExpr arguments)
{
  ListExpr type = NList::typeError("Operator map2 expects three arguments.");

  if(arguments != 0)
  {
    NList argumentsList(arguments);

    if(argumentsList.hasLength(3))
    {
      std::string argument1 = argumentsList.first().str();

      if(IstType(argument1) ||
         IsmtType(argument1))
      {
        std::string argument2 = argumentsList.second().str();

        if(IstType(argument2) ||
           IsmtType(argument2))
        {
          NList argument3 = argumentsList.third();

          if (listutils::isMap<2>(argument3.listExpr()))
          {
            std::string valueWrapperType1 = GetValueWrapperType(argument1);
            std::string functionArgument1 = argument3.second().str();

            if(valueWrapperType1 == functionArgument1)
            {
              std::string valueWrapperType2 = GetValueWrapperType(argument2);
              std::string functionArgument2 = argument3.third().str();

              if(valueWrapperType2 == functionArgument2)
              {
                std::string functionResult = argument3.fourth().str();
                std::string typeName;

                if(IstType(argument1) &&
                   IstType(argument2))
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
                                          "is not a t type or a mt type.");
                }
              }

              else
              {
                type = NList::typeError("ValueWrapperType2 and "
                                        "function argument2 does not match.");
              }
            }

            else
            {
              type = NList::typeError("ValueWrapperType1 and "
                                      "function argument1 does not match.");
            }
          }

          else
          {
            type = NList::typeError("Third argument must be a function "
                                    "with two arguments.");
          }
        }

        else
        {
          type = NList::typeError("Second argument must be "
                                  "a t type or a mt type.");
        }
      }

      else
      {
        type = NList::typeError("First argument must be "
                                "a t type or a mt type.");
      }
    }
  }

  return type;
}

}
