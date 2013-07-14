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

#include "toraster2.h"
#include "../t/tint.h"
#include "../t/treal.h"
#include "../t/tbool.h"
#include "../t/tstring.h"
#include "../mt/mtint.h"
#include "../mt/mtreal.h"
#include "../mt/mtbool.h"
#include "../mt/mtstring.h"
#include "../../Raster2/grid2.h"
#include "../../Raster2/grid3.h"
#include "../../Raster2/sint.h"
#include "../../Raster2/sreal.h"
#include "../../Raster2/sbool.h"
#include "../../Raster2/sstring.h"
#include "../../Raster2/msint.h"
#include "../../Raster2/msreal.h"
#include "../../Raster2/msbool.h"
#include "../../Raster2/msstring.h"
#include "DateTime.h"

namespace TileAlgebra
{

/*
definition of template toraster2Functiont

*/

template <typename SourceType, typename SourceTypeProperties,
          typename DestinationType>
int toraster2Functiont(Word* pArguments,
                       Word& rResult,
                       int message,
                       Word& rLocal,
                       Supplier supplier)
{
  int nRetVal = 0;

  if(qp != 0 &&
     pArguments != 0)
  {
    rResult = qp->ResultStorage(supplier);

    if(rResult.addr != 0)
    {
      DestinationType* pDestinationType = static_cast<DestinationType*>
                                         (rResult.addr);

      if(pDestinationType != 0)
      {
        pDestinationType->setDefined(false);

        switch(message)
        {
          case OPEN:
          case REQUEST:
          case CLOSE:
          {
            qp->Open(pArguments[0].addr);

            raster2::grid2 grid(std::numeric_limits<double>::max(),
                                std::numeric_limits<double>::max(),
                                std::numeric_limits<double>::max());
            int xDimensionSize = SourceTypeProperties::GetXDimensionSize();
            int yDimensionSize = SourceTypeProperties::GetYDimensionSize();

            Word word;
            qp->Request(pArguments[0].addr, word);

            while(qp->Received(pArguments[0].addr))
            {
              SourceType* pSourceType = static_cast<SourceType*>(word.addr);

              if(pSourceType != 0)
              {
                pDestinationType->setDefined(true);

                tgrid sourceGrid;
                pSourceType->getgrid(sourceGrid);

                double sourceGridX = sourceGrid.GetX();
                double sourceGridY = sourceGrid.GetY();
                double sourceGridLength = sourceGrid.GetLength();

                if(sourceGridX < grid.getOriginX() &&
                   sourceGridY < grid.getOriginY())
                {
                  // set smallest x and y coordinates
                  grid.set(sourceGridX, sourceGridY, sourceGridLength);
                  pDestinationType->setGrid(grid);
                }

                for(int row = 0; row < yDimensionSize; row++)
                {
                  for(int column = 0; column < xDimensionSize; column++)
                  {
                    double x = sourceGridX + column * sourceGridLength;
                    double y = sourceGridY + row * sourceGridLength;
                    typename SourceTypeProperties::TypeProperties::
                    WrapperType wrappedValue;

                    pSourceType->atlocation(x, y, wrappedValue);

                    if(wrappedValue.IsDefined())
                    {
                      typename SourceTypeProperties::TypeProperties::
                      PropertiesType value = SourceTypeProperties::
                      TypeProperties::GetUnwrappedValue(wrappedValue);

                      if(SourceTypeProperties::TypeProperties::
                         IsUndefinedValue(value) == false)
                      {
                        pDestinationType->setatlocation(x, y, value);
                      }
                    }
                  }
                }

                pSourceType->DeleteIfAllowed();
              }

              qp->Request(pArguments[0].addr, word);
            }

            qp->Close(pArguments[0].addr);
          }
          break;

          default:
          {
            assert(false);
            nRetVal = -1;
          }
          break;
        }
      }
    }
  }

  return nRetVal;
}

/*
definition of template toraster2Functionmt

*/

template <typename SourceType, typename SourceTypeProperties,
          typename DestinationType>
int toraster2Functionmt(Word* pArguments,
                        Word& rResult,
                        int message,
                        Word& rLocal,
                        Supplier supplier)
{
  int nRetVal = 0;

  if(qp != 0 &&
     pArguments != 0)
  {
    rResult = qp->ResultStorage(supplier);

    if(rResult.addr != 0)
    {
      DestinationType* pDestinationType = static_cast<DestinationType*>
                                         (rResult.addr);

      if(pDestinationType != 0)
      {
        pDestinationType->setDefined(false);

        switch(message)
        {
          case OPEN:
          case REQUEST:
          case CLOSE:
          {
            qp->Open(pArguments[0].addr);

            raster2::grid3 grid(std::numeric_limits<double>::max(),
                                std::numeric_limits<double>::max(),
                                std::numeric_limits<double>::max(),
                                datetime::DateTime(0.0));
            int xDimensionSize = SourceTypeProperties::GetXDimensionSize();
            int yDimensionSize = SourceTypeProperties::GetYDimensionSize();
            int tDimensionSize = SourceTypeProperties::GetTDimensionSize();
            Word word;
            qp->Request(pArguments[0].addr, word);

            while(qp->Received(pArguments[0].addr))
            {
              SourceType* pSourceType = static_cast<SourceType*>(word.addr);

              if(pSourceType != 0)
              {
                pDestinationType->setDefined(true);

                mtgrid sourceGrid;
                pSourceType->getgrid(sourceGrid);

                double sourceGridX = sourceGrid.GetX();
                double sourceGridY = sourceGrid.GetY();
                double sourceGridLength = sourceGrid.GetLength();
                datetime::DateTime sourceGridDuration = sourceGrid.
                                                        GetDuration();

                if(sourceGridX < grid.getOriginX() &&
                   sourceGridY < grid.getOriginY())
                {
                  // set smallest x and y coordinates
                  grid.set(sourceGridX, sourceGridY,
                           sourceGridLength, sourceGridDuration);
                  pDestinationType->setGrid(grid);
                }

                for(int time = 0; time < tDimensionSize; time++)
                {
                  for(int row = 0; row < yDimensionSize; row++)
                  {
                    for(int column = 0; column < xDimensionSize; column++)
                    {
                      double x = sourceGridX + column * sourceGridLength;
                      double y = sourceGridY + row * sourceGridLength;
                      double t = time * sourceGridDuration.ToDouble();
                      typename SourceTypeProperties::TypeProperties::
                      WrapperType wrappedValue;

                      pSourceType->atlocation(x, y, t, wrappedValue);

                      if(wrappedValue.IsDefined())
                      {
                        typename SourceTypeProperties::TypeProperties::
                        PropertiesType value = SourceTypeProperties::
                        TypeProperties::GetUnwrappedValue(wrappedValue);

                        if(SourceTypeProperties::TypeProperties::
                           IsUndefinedValue(value) == false)
                        {
                          pDestinationType->setatlocation(x, y, t, value);
                        }
                      }
                    }
                  }
                }

                pSourceType->DeleteIfAllowed();
              }

              qp->Request(pArguments[0].addr, word);
            }

            qp->Close(pArguments[0].addr);
          }
          break;

          default:
          {
            assert(false);
            nRetVal = -1;
          }
          break;
        }
      }
    }
  }

  return nRetVal;
}

/*
definition of toraster2 functions

*/

ValueMapping toraster2Functions[] =
{
  toraster2Functiont<tint, tProperties<int>, raster2::sint>,
  toraster2Functiont<treal, tProperties<double>,raster2::sreal>,
  toraster2Functiont<tbool, tProperties<char>,raster2::sbool>,
  toraster2Functiont<tstring, tProperties<std::string>,raster2::sstring>,
  toraster2Functionmt<mtint, mtProperties<int>,raster2::msint>,
  toraster2Functionmt<mtreal, mtProperties<double>,raster2::msreal>,
  toraster2Functionmt<mtbool, mtProperties<char>,raster2::msbool>,
  toraster2Functionmt<mtstring, mtProperties<std::string>,raster2::msstring>,
  0
};

/*
definition of toraster2 select function

*/

int toraster2SelectFunction(ListExpr arguments)
{
  int nSelection = -1;

  if(arguments != 0)
  {
    NList argumentsList(arguments);

    if(argumentsList.hasLength(1))
    {
      NList argument2 = argumentsList.first().second();
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
          nSelection = i;
          break;
        }
      }
    }
  }

  return nSelection;
}

/*
definition of toraster2 type mapping function

*/

ListExpr toraster2TypeMappingFunction(ListExpr arguments)
{
  ListExpr type = NList::typeError("Operator toraster2 expects "
                                   "a stream of t type objects or "
                                   "a stream of mt type objects.");

  if(nl != 0)
  {
    NList argumentsList(arguments);

    if(argumentsList.hasLength(1))
    {
      NList argument1 = argumentsList.first();

      if(Stream<Attribute>::checkType(argument1.listExpr()))
      {
        std::string typeName = argument1.second().str();

        if(typeName.empty() == false)
        {
          if(IstType(typeName))
          {
            type = NList(GetsType(typeName)).listExpr();
          }

          else if(IsmtType(typeName))
          {
            type = NList(GetmsType(typeName)).listExpr();
          }
        }
      }
    }
  }

  return type;
}

}
