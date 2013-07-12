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

#include "tiles.h"
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
#include "Stream.h"

namespace TileAlgebra
{

/*
definition of template tilesFunctiont

*/

template <typename SourceType,
          typename DestinationType, typename DestinationTypeProperties>
int tilesFunctiont(Word* pArguments,
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

    if(pSourceType != 0)
    {
      struct ConvertInfo
      {
        Rectangle<2> m_BoundingBox;
        double m_dX;
        double m_dY;
      };

      switch(message)
      {
        case OPEN:
        {
          // initialize the local storage
          ConvertInfo* pConvertInfo = new ConvertInfo;

          if(pConvertInfo != 0)
          {
            pConvertInfo->m_BoundingBox = pSourceType->bbox();
            pConvertInfo->m_dX = pConvertInfo->m_BoundingBox.MinD(0);
            pConvertInfo->m_dY = pConvertInfo->m_BoundingBox.MinD(1);
            rLocal.addr = pConvertInfo;
          }
        }
        break;

        case REQUEST:
        {
          if(rLocal.addr != 0)
          {
            ConvertInfo* pConvertInfo = static_cast<ConvertInfo*>(rLocal.addr);

            if(pConvertInfo != 0)
            {
              // return the next stream element

              if(pConvertInfo->m_dX < pConvertInfo->m_BoundingBox.MaxD(0) &&
                 pConvertInfo->m_dY < pConvertInfo->m_BoundingBox.MaxD(1))
              {
                DestinationType* pDestinationType = new DestinationType(true);

                if(pDestinationType != 0)
                {
                  raster2::grid2 grid = pSourceType->getGrid();
                  double gridLength = grid.getLength();
                  int xDimensionSize = DestinationTypeProperties::
                                       GetXDimensionSize();
                  int yDimensionSize = DestinationTypeProperties::
                                       GetYDimensionSize();
                  bool bHasDefinedValue = false;

                  do
                  {
                    pDestinationType->SetGrid(pConvertInfo->m_dX,
                                              pConvertInfo->m_dY,
                                              gridLength);
                    
                    for(int row = 0; row < yDimensionSize; row++)
                    {
                      for(int column = 0; column < xDimensionSize; column++)
                      {
                        double x = pConvertInfo->m_dX + column * gridLength;
                        double y = pConvertInfo->m_dY + row * gridLength;
                        
                        typename DestinationTypeProperties::TypeProperties::
                        PropertiesType value = pSourceType->atlocation(x, y);

                        if(DestinationTypeProperties::TypeProperties::
                           IsUndefinedValue(value) == false)
                        {
                          bHasDefinedValue = true;
                          pDestinationType->SetValue(x, y, value, true);
                        }
                      }
                    }

                    pConvertInfo->m_dX += xDimensionSize * gridLength;

                    if(pConvertInfo->m_dX >=
                       pConvertInfo->m_BoundingBox.MaxD(0))
                    {
                      pConvertInfo->m_dY += yDimensionSize * gridLength;

                      if(pConvertInfo->m_dY <
                         pConvertInfo->m_BoundingBox.MaxD(1))
                      {
                        pConvertInfo->m_dX =
                        pConvertInfo->m_BoundingBox.MinD(0);
                      }
                    }
                  }

                  while(bHasDefinedValue == false);

                  rResult.addr = pDestinationType;
                  nRetVal = YIELD;
                }
              }

              else
              {
                // always set the result to null before return CANCEL
                rResult.addr = 0;
                nRetVal = CANCEL;
              }
            }
          }
        }
        break;

        case CLOSE:
        {
          if(rLocal.addr != 0)
          {
            ConvertInfo* pConvertInfo = static_cast<ConvertInfo*>(rLocal.addr);

            if(pConvertInfo != 0)
            {
              delete pConvertInfo;
              rLocal.addr = 0;
            }
          }
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

  return nRetVal;
}

/*
definition of template tilesFunctionmt

*/

template <typename SourceType,
          typename DestinationType, typename DestinationTypeProperties>
int tilesFunctionmt(Word* pArguments,
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

    if(pSourceType != 0)
    {
      struct ConvertInfo
      {
        Rectangle<3> m_BoundingBox;
        double m_dX;
        double m_dY;
        double m_dT;
      };

      switch(message)
      {
        case OPEN:
        {
          // initialize the local storage
          ConvertInfo* pConvertInfo = new ConvertInfo;

          if(pConvertInfo != 0)
          {
            pConvertInfo->m_BoundingBox = pSourceType->bbox();
            pConvertInfo->m_dX = pConvertInfo->m_BoundingBox.MinD(0);
            pConvertInfo->m_dY = pConvertInfo->m_BoundingBox.MinD(1);
            pConvertInfo->m_dT = pConvertInfo->m_BoundingBox.MinD(2);
            rLocal.addr = pConvertInfo;
          }
        }
        break;

        case REQUEST:
        {
          if(rLocal.addr != 0)
          {
            ConvertInfo* pConvertInfo = static_cast<ConvertInfo*>(rLocal.addr);

            if(pConvertInfo != 0)
            {
              // return the next stream element

              if(pConvertInfo->m_dX < pConvertInfo->m_BoundingBox.MaxD(0) &&
                 pConvertInfo->m_dY < pConvertInfo->m_BoundingBox.MaxD(1) &&
                 pConvertInfo->m_dT < pConvertInfo->m_BoundingBox.MaxD(2))
              {
                DestinationType* pDestinationType = new DestinationType(true);

                if(pDestinationType != 0)
                {
                  raster2::grid3 grid = pSourceType->getGrid();
                  double gridLength = grid.getLength();
                  double gridDuration = grid.getDuration().ToDouble();
                  int xDimensionSize = DestinationTypeProperties::
                                       GetXDimensionSize();
                  int yDimensionSize = DestinationTypeProperties::
                                       GetYDimensionSize();
                  int tDimensionSize = DestinationTypeProperties::
                                       GetTDimensionSize();
                  bool bHasDefinedValue = false;

                  do
                  {
                    pDestinationType->SetGrid(pConvertInfo->m_dX,
                                              pConvertInfo->m_dY,
                                              gridLength,
                                              gridDuration);

                    for(int time = 0; time < tDimensionSize; time++)
                    {
                      for(int row = 0; row < yDimensionSize; row++)
                      {
                        for(int column = 0; column < xDimensionSize; column++)
                        {
                          double x = pConvertInfo->m_dX + column * gridLength;
                          double y = pConvertInfo->m_dY + row * gridLength;
                          double t = time * gridDuration;
                          
                          typename DestinationTypeProperties::TypeProperties::
                          PropertiesType value =
                          pSourceType->atlocation(x, y, t);

                          if(DestinationTypeProperties::TypeProperties::
                             IsUndefinedValue(value) == false)
                          {
                            bHasDefinedValue = true;
                            pDestinationType->SetValue(x, y, t, value, true);
                          }
                        }
                      }
                    }

                    pConvertInfo->m_dX += xDimensionSize * gridLength;

                    if(pConvertInfo->m_dX >=
                       pConvertInfo->m_BoundingBox.MaxD(0))
                    {
                      pConvertInfo->m_dY += yDimensionSize * gridLength;

                      if(pConvertInfo->m_dY >=
                         pConvertInfo->m_BoundingBox.MaxD(1))
                      {
                        pConvertInfo->m_dT += tDimensionSize * gridDuration;

                        if(pConvertInfo->m_dT <
                           pConvertInfo->m_BoundingBox.MaxD(2))
                        {
                          pConvertInfo->m_dX =
                          pConvertInfo->m_BoundingBox.MinD(0);
                          pConvertInfo->m_dY =
                          pConvertInfo->m_BoundingBox.MinD(1);
                        }
                      }

                      else
                      {
                        pConvertInfo->m_dX =
                        pConvertInfo->m_BoundingBox.MinD(0);
                      }
                    }
                  }

                  while(bHasDefinedValue == false);

                  rResult.addr = pDestinationType;
                  nRetVal = YIELD;
                }
              }

              else
              {
                // always set the result to null before return CANCEL
                rResult.addr = 0;
                nRetVal = CANCEL;
              }
            }
          }
        }
        break;

        case CLOSE:
        {
          if(rLocal.addr != 0)
          {
            ConvertInfo* pConvertInfo = static_cast<ConvertInfo*>(rLocal.addr);

            if(pConvertInfo != 0)
            {
              delete pConvertInfo;
              rLocal.addr = 0;
            }
          }
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

  return nRetVal;
}

/*
definition of tiles functions

*/

ValueMapping tilesFunctions[] =
{
  tilesFunctiont<raster2::sint, tint, tProperties<int> >,
  tilesFunctiont<raster2::sreal, treal, tProperties<double> >,
  tilesFunctiont<raster2::sbool, tbool, tProperties<char> >,
  tilesFunctiont<raster2::sstring, tstring, tProperties<std::string> >,
  tilesFunctionmt<raster2::msint, mtint, mtProperties<int> >,
  tilesFunctionmt<raster2::msreal, mtreal, mtProperties<double> >,
  tilesFunctionmt<raster2::msbool, mtbool, mtProperties<char> >,
  tilesFunctionmt<raster2::msstring, mtstring, mtProperties<std::string> >,
  0
};

/*
definition of tiles select function

*/

int tilesSelectFunction(ListExpr arguments)
{
  int nSelection = -1;

  if(arguments != 0)
  {
    NList argumentsList(arguments);

    if(argumentsList.hasLength(1))
    {
      NList argument1 = argumentsList.first();
      const int TYPE_NAMES = 8;
      const std::string TYPE_NAMES_ARRAY[TYPE_NAMES] =
      {
        raster2::sint::BasicType(),
        raster2::sreal::BasicType(),
        raster2::sbool::BasicType(),
        raster2::sstring::BasicType(),
        raster2::msint::BasicType(),
        raster2::msreal::BasicType(),
        raster2::msbool::BasicType(),
        raster2::msstring::BasicType()
      };

      for(int i = 0; i < TYPE_NAMES; i++)
      {
        if(argument1.isSymbol(TYPE_NAMES_ARRAY[i]))
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
definition of tiles type mapping function

*/

ListExpr tilesTypeMappingFunction(ListExpr arguments)
{
  ListExpr type = NList::typeError("Operator tiles expects "
                                   "a s type or a ms type.");

  if(nl != 0)
  {
    NList argumentsList(arguments);

    if(argumentsList.hasLength(1))
    {
      std::string argument1 = argumentsList.first().str();

      if(IssType(argument1))
      {
        type = nl->TwoElemList(nl->SymbolAtom(Stream<Attribute>::BasicType()),
                               nl->SymbolAtom(GettType(argument1)));
      }

      else if(IsmsType(argument1))
      {
        type = nl->TwoElemList(nl->SymbolAtom(Stream<Attribute>::BasicType()),
                               nl->SymbolAtom(GetmtType(argument1)));
      }
    }
  }

  return type;
}

}
