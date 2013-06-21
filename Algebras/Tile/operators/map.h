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

#ifndef TILEALGEBRA_MAP_H
#define TILEALGEBRA_MAP_H

#include "AlgebraTypes.h"
#include "Operator.h"
#include "QueryProcessor.h"
#include "../grid/tgrid.h"
#include "../grid/mtgrid.h"
#include "../Index.h"
#include "RectangleAlgebra.h"

namespace TileAlgebra
{

/*
declaration of map functions

*/

extern ValueMapping mapFunctions[];

/*
declaration of map select function

*/

int mapSelectFunction(ListExpr arguments);

/*
declaration of map type mapping function

*/

ListExpr mapTypeMappingFunction(ListExpr arguments);

/*
definition of map Operator Info structure

*/

struct mapInfo : OperatorInfo
{
  mapInfo()
  {
    name      = "map";
    signature = "xT x (T x U) -> xU";
    syntax    = "_ map[_]";
    meaning   = "Maps a xT type to a different xU type.";
  }
};

/*
definition of template mapFunctiont

*/

template <typename SourceType, typename SourceTypeProperties,
          typename DestinationType, typename DestinationTypeProperties>
int mapFunctiont(Word* pArguments,
                 Word& rResult,
                 int message,
                 Word& rLocal,
                 Supplier supplier)
{
  int nRetVal = 0;

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
        DestinationType* pResult =static_cast<DestinationType*>(rResult.addr);
        
        if(pResult != 0)
        {
          typename SourceTypeProperties::bboxType boundingBox;
          pSourceType->bbox(boundingBox);

          if(boundingBox.IsDefined())
          {
            tgrid grid;
            pSourceType->getgrid(grid);
            pResult->SetGrid(grid.GetX(), grid.GetY(), grid.GetLength());

            int xDimensionSize = SourceTypeProperties::GetXDimensionSize();
            int yDimensionSize = SourceTypeProperties::GetYDimensionSize();
            ArgVector& argumentsVector = *qp->Argument(pFunction);
            Word word;

            for(int row = 0; row < yDimensionSize; row++)
            {
              for(int column = 0; column < xDimensionSize; column++)
              {
                Index<2> index = (int[]){column, row};
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

                  typename DestinationTypeProperties::TypeProperties::
                  WrapperType& mappedValue = *(static_cast <typename
                  DestinationTypeProperties::TypeProperties::WrapperType*>
                  (word.addr));

                  typename DestinationTypeProperties::TypeProperties::
                  PropertiesType unwrappedValue = DestinationTypeProperties::
                  TypeProperties::GetUnwrappedValue(mappedValue);

                  if(DestinationTypeProperties::TypeProperties::
                     IsUndefinedValue(unwrappedValue) == false)
                  {
                    pResult->SetValue(index, unwrappedValue, true);
                  }
                }
              }
            }
          }

          else
          {
            pResult->SetDefined(false);
          }
        }
      }
    }
  }

  return nRetVal;
}

/*
definition of template mapFunctionmt

*/

template <typename SourceType, typename SourceTypeProperties,
          typename DestinationType, typename DestinationTypeProperties>
int mapFunctionmt(Word* pArguments,
                  Word& rResult,
                  int message,
                  Word& rLocal,
                  Supplier supplier)
{
  int nRetVal = 0;

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
        DestinationType* pResult =static_cast<DestinationType*>(rResult.addr);
        
        if(pResult != 0)
        {
          typename SourceTypeProperties::bboxType boundingBox;
          pSourceType->bbox(boundingBox);

          if(boundingBox.IsDefined())
          {
            mtgrid grid;
            pSourceType->getgrid(grid);
            pResult->SetGrid(grid.GetX(), grid.GetY(),
                             grid.GetLength(), grid.GetDuration());

            int xDimensionSize = SourceTypeProperties::GetXDimensionSize();
            int yDimensionSize = SourceTypeProperties::GetYDimensionSize();
            int tDimensionSize = SourceTypeProperties::GetTDimensionSize();
            ArgVector& argumentsVector = *qp->Argument(pFunction);
            Word word;

            for(int time = 0; time < tDimensionSize; time++)
            {
              for(int row = 0; row < yDimensionSize; row++)
              {
                for(int column = 0; column < xDimensionSize; column++)
                {
                  Index<3> index = (int[]){column, row, time};
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

                    typename DestinationTypeProperties::TypeProperties::
                    WrapperType& mappedValue = *(static_cast<typename
                    DestinationTypeProperties::TypeProperties::WrapperType*>
                    (word.addr));

                    typename DestinationTypeProperties::TypeProperties::
                    PropertiesType unwrappedValue = DestinationTypeProperties::
                    TypeProperties::GetUnwrappedValue(mappedValue);

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

          else
          {
            pResult->SetDefined(false);
          }
        }
      }
    }
  }

  return nRetVal;
}

}

#endif // TILEALGEBRA_MAP_H
