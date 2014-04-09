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
Raster2Algebra includes

*/

#include "../../Raster2/sint.h"
#include "../../Raster2/sreal.h"
#include "../../Raster2/sbool.h"
#include "../../Raster2/sstring.h"
#include "../../Raster2/msint.h"
#include "../../Raster2/msreal.h"
#include "../../Raster2/msbool.h"
#include "../../Raster2/msstring.h"
#include "../../Raster2/isint.h"
#include "../../Raster2/isreal.h"
#include "../../Raster2/isbool.h"
#include "../../Raster2/isstring.h"

/*
TileAlgebra includes

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
#include "../it/itint.h"
#include "../it/itreal.h"
#include "../it/itbool.h"
#include "../it/itstring.h"

/*
declaration of namespace TileAlgebra

*/

namespace TileAlgebra
{

/*
Method tilesFunctiontgrid converts a Raster2 Algebra grid2 object
to a Tile Algebra tgrid object.

author: Dirk Zacher
parameters: pArguments - a pointer to the arguments of tiles operator
            rResult - reference to a Word containing the result
            message - message to distinguish call modes of tilesFunctiontgrid
            rLocal - reference to a Word to store local method information
            supplier - an Address to a supplier of information of operator tree
return value: 0 if tilesFunctiontgrid successfully executed, otherwise FAILURE
exceptions: -

*/

int tilesFunctiontgrid(Word* pArguments,
                       Word& rResult,
                       int message,
                       Word& rLocal,
                       Supplier supplier)
{
  int nRetVal = FAILURE;

  if(qp != 0 &&
     pArguments != 0)
  {
    raster2::grid2* pGrid = static_cast<raster2::grid2*>(pArguments[0].addr);

    if(pGrid != 0)
    {
      rResult = qp->ResultStorage(supplier);

      if(rResult.addr != 0)
      {
        tgrid* pResult = static_cast<tgrid*>(rResult.addr);

        if(pResult != 0)
        {
          pResult->SetX(pGrid->getOriginX());
          pResult->SetY(pGrid->getOriginY());
          pResult->SetLength(pGrid->getLength());

          nRetVal = 0;
        }
      }
    }
  }

  return nRetVal;
}

/*
Method tilesFunctionmtgrid converts a Raster2 Algebra grid3 object
to a Tile Algebra mtgrid object.

author: Dirk Zacher
parameters: pArguments - a pointer to the arguments of tiles operator
            rResult - reference to a Word containing the result
            message - message to distinguish call modes of tilesFunctionmtgrid
            rLocal - reference to a Word to store local method information
            supplier - an Address to a supplier of information of operator tree
return value: 0 if tilesFunctionmtgrid successfully executed, otherwise FAILURE
exceptions: -

*/

int tilesFunctionmtgrid(Word* pArguments,
                        Word& rResult,
                        int message,
                        Word& rLocal,
                        Supplier supplier)
{
  int nRetVal = FAILURE;

  if(qp != 0 &&
     pArguments != 0)
  {
    raster2::grid3* pGrid = static_cast<raster2::grid3*>(pArguments[0].addr);

    if(pGrid != 0)
    {
      rResult = qp->ResultStorage(supplier);

      if(rResult.addr != 0)
      {
        mtgrid* pResult = static_cast<mtgrid*>(rResult.addr);

        if(pResult != 0)
        {
          pResult->SetX(pGrid->getOriginX());
          pResult->SetY(pGrid->getOriginY());
          pResult->SetLength(pGrid->getLength());
          pResult->SetDuration(pGrid->getDuration());

          nRetVal = 0;
        }
      }
    }
  }

  return nRetVal;
}

/*
Template method tilesFunctiont converts a Raster2 Algebra s type object
to a stream of Tile Algebra t type objects.

author: Dirk Zacher
parameters: pArguments - a pointer to the arguments of tiles operator
            rResult - reference to a Word containing the result
            message - message to distinguish call modes of tilesFunctiont
            rLocal - reference to a Word to store local method information
            supplier - an Address to a supplier of information of operator tree
return value: 0 if tilesFunctiont successfully executed,
              YIELD if rResult contains a stream element (execution continued),
              CANCEL if all stream elements of the result already returned,
              FAILURE if an error occured
exceptions: -

*/

template <typename SourceType,
          typename DestinationType, typename DestinationTypeProperties>
int tilesFunctiont(Word* pArguments,
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

    if(pSourceType != 0 &&
       pSourceType->isDefined())
    {
      struct ResultInfo
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
          ResultInfo* pResultInfo = new ResultInfo;

          if(pResultInfo != 0)
          {
            pResultInfo->m_BoundingBox = pSourceType->bbox();
            pResultInfo->m_dX = pResultInfo->m_BoundingBox.MinD(0);
            pResultInfo->m_dY = pResultInfo->m_BoundingBox.MinD(1);
            rLocal.addr = pResultInfo;
            nRetVal = 0;
          }
        }
        break;

        case REQUEST:
        {
          if(rLocal.addr != 0)
          {
            ResultInfo* pResultInfo = static_cast<ResultInfo*>(rLocal.addr);

            if(pResultInfo != 0)
            {
              if(pResultInfo->m_dX < pResultInfo->m_BoundingBox.MaxD(0) &&
                 pResultInfo->m_dY < pResultInfo->m_BoundingBox.MaxD(1))
              {
                DestinationType* pDestinationType = new DestinationType(true);

                if(pDestinationType != 0)
                {
                  raster2::grid2 grid = pSourceType->getGrid();
                  double gridLength = grid.getLength();
                  double halfGridLength = gridLength / 2.0;
                  int xDimensionSize = DestinationTypeProperties::
                                       GetXDimensionSize();
                  int yDimensionSize = DestinationTypeProperties::
                                       GetYDimensionSize();
                  bool bHasDefinedValue = false;

                  do
                  {
                    pDestinationType->SetGrid(pResultInfo->m_dX,
                                              pResultInfo->m_dY,
                                              gridLength);
                    
                    for(int row = 0; row < yDimensionSize; row++)
                    {
                      for(int column = 0; column < xDimensionSize; column++)
                      {
                        double x = pResultInfo->m_dX + column * gridLength +
                                   halfGridLength;
                        double y = pResultInfo->m_dY + row * gridLength +
                                   halfGridLength;
                        
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

                    pResultInfo->m_dX += xDimensionSize * gridLength;

                    if(pResultInfo->m_dX >=
                       pResultInfo->m_BoundingBox.MaxD(0))
                    {
                      pResultInfo->m_dY += yDimensionSize * gridLength;

                      if(pResultInfo->m_dY <
                         pResultInfo->m_BoundingBox.MaxD(1))
                      {
                        pResultInfo->m_dX =
                        pResultInfo->m_BoundingBox.MinD(0);
                      }
                      
                      else
                      {
                        break;
                      }
                    }
                  }

                  while(bHasDefinedValue == false);
                  
                  if(bHasDefinedValue == true)
                  {
                    // return the next stream element
                    rResult.addr = pDestinationType;
                    nRetVal = YIELD;
                  }
                  
                  else
                  {
                    delete pDestinationType;
                    pDestinationType = 0;
                    
                    // always set the result to null before return CANCEL
                    rResult.addr = 0;
                    nRetVal = CANCEL;
                  }
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
            ResultInfo* pResultInfo = static_cast<ResultInfo*>(rLocal.addr);

            if(pResultInfo != 0)
            {
              delete pResultInfo;
              rLocal.addr = 0;
            }
          }

          nRetVal = 0;
        }
        break;

        default:
        {
          assert(false);
        }
        break;
      }
    }
  }

  return nRetVal;
}

/*
Template method tilesFunctionmt converts a Raster2 Algebra ms type object
to a stream of Tile Algebra mt type objects.

author: Dirk Zacher
parameters: pArguments - a pointer to the arguments of tiles operator
            rResult - reference to a Word containing the result
            message - message to distinguish call modes of tilesFunctionmt
            rLocal - reference to a Word to store local method information
            supplier - an Address to a supplier of information of operator tree
return value: 0 if tilesFunctionmt successfully executed,
              YIELD if rResult contains a stream element (execution continued),
              CANCEL if all stream elements of the result already returned,
              FAILURE if an error occured
exceptions: -

*/

template <typename SourceType,
          typename DestinationType, typename DestinationTypeProperties>
int tilesFunctionmt(Word* pArguments,
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

    if(pSourceType != 0)
    {
      struct ResultInfo
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
          ResultInfo* pResultInfo = new ResultInfo;

          if(pResultInfo != 0)
          {
            pResultInfo->m_BoundingBox = pSourceType->bbox();
            pResultInfo->m_dX = pResultInfo->m_BoundingBox.MinD(0);
            pResultInfo->m_dY = pResultInfo->m_BoundingBox.MinD(1);
            pResultInfo->m_dT = pResultInfo->m_BoundingBox.MinD(2);
            rLocal.addr = pResultInfo;
            nRetVal = 0;
          }
        }
        break;

        case REQUEST:
        {
          if(rLocal.addr != 0)
          {
            ResultInfo* pResultInfo = static_cast<ResultInfo*>(rLocal.addr);

            if(pResultInfo != 0)
            {
              // return the next stream element

              if(pResultInfo->m_dX < pResultInfo->m_BoundingBox.MaxD(0) &&
                 pResultInfo->m_dY < pResultInfo->m_BoundingBox.MaxD(1) &&
                 pResultInfo->m_dT < pResultInfo->m_BoundingBox.MaxD(2))
              {
                DestinationType* pDestinationType = new DestinationType(true);

                if(pDestinationType != 0)
                {
                  raster2::grid3 grid = pSourceType->getGrid();
                  double gridLength = grid.getLength();
                  double halfGridLength = gridLength / 2.0;
                  double gridDuration = grid.getDuration().ToDouble();
                  double halfGridDuration = gridDuration / 2.0;
                  int xDimensionSize = DestinationTypeProperties::
                                       GetXDimensionSize();
                  int yDimensionSize = DestinationTypeProperties::
                                       GetYDimensionSize();
                  int tDimensionSize = DestinationTypeProperties::
                                       GetTDimensionSize();
                  bool bHasDefinedValue = false;

                  do
                  {
                    pDestinationType->SetGrid(pResultInfo->m_dX,
                                              pResultInfo->m_dY,
                                              gridLength,
                                              gridDuration);
                    pDestinationType->SetGridT(pResultInfo->m_dT);

                    for(int time = 0; time < tDimensionSize; time++)
                    {
                      for(int row = 0; row < yDimensionSize; row++)
                      {
                        for(int column = 0; column < xDimensionSize; column++)
                        {
                          double x = pResultInfo->m_dX + column * gridLength +
                                     halfGridLength;
                          double y = pResultInfo->m_dY + row * gridLength +
                                     halfGridLength;
                          double t = pResultInfo->m_dT + time * gridDuration +
                                     halfGridDuration;
                          
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

                    pResultInfo->m_dX += xDimensionSize * gridLength;

                    if(pResultInfo->m_dX >=
                       pResultInfo->m_BoundingBox.MaxD(0))
                    {
                      pResultInfo->m_dY += yDimensionSize * gridLength;

                      if(pResultInfo->m_dY >=
                         pResultInfo->m_BoundingBox.MaxD(1))
                      {
                        pResultInfo->m_dT += tDimensionSize * gridDuration;

                        if(pResultInfo->m_dT <
                           pResultInfo->m_BoundingBox.MaxD(2))
                        {
                          pResultInfo->m_dX =
                          pResultInfo->m_BoundingBox.MinD(0);
                          pResultInfo->m_dY =
                          pResultInfo->m_BoundingBox.MinD(1);
                        }
                        
                        else
                        {
                          break;
                        }
                      }

                      else
                      {
                        pResultInfo->m_dX =
                        pResultInfo->m_BoundingBox.MinD(0);
                      }
                    }
                  }

                  while(bHasDefinedValue == false);
                  
                  if(bHasDefinedValue == true)
                  {
                    rResult.addr = pDestinationType;
                    nRetVal = YIELD;
                  }
                  
                  else
                  {
                    delete pDestinationType;
                    pDestinationType = 0;
                    
                    // always set the result to null before return CANCEL
                    rResult.addr = 0;
                    nRetVal = CANCEL;
                  }
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
            ResultInfo* pResultInfo = static_cast<ResultInfo*>(rLocal.addr);

            if(pResultInfo != 0)
            {
              delete pResultInfo;
              rLocal.addr = 0;
            }
          }

          nRetVal = 0;
        }
        break;

        default:
        {
          assert(false);
        }
        break;
      }
    }
  }

  return nRetVal;
}

/*
Template method tilesFunctionit converts a Raster2 Algebra is type object
to a stream of Tile Algebra it type objects.

author: Dirk Zacher
parameters: pArguments - a pointer to the arguments of tiles operator
            rResult - reference to a Word containing the result
            message - message to distinguish call modes of tilesFunctionit
            rLocal - reference to a Word to store local method information
            supplier - an Address to a supplier of information of operator tree
return value: 0 if tilesFunctionit successfully executed,
              YIELD if rResult contains a stream element (execution continued),
              CANCEL if all stream elements of the result already returned,
              FAILURE if an error occured
exceptions: -

*/

template <typename SourceType, typename SourceTypeProperties,
          typename DestinationType, typename DestinationTypeProperties>
int tilesFunctionit(Word* pArguments,
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

    if(pSourceType != 0 &&
       pSourceType->isDefined())
    {
      struct ResultInfo
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
          ResultInfo* pResultInfo = new ResultInfo;

          if(pResultInfo != 0)
          {
            pResultInfo->m_BoundingBox = pSourceType->bbox();
            pResultInfo->m_dX = pResultInfo->m_BoundingBox.MinD(0);
            pResultInfo->m_dY = pResultInfo->m_BoundingBox.MinD(1);
            rLocal.addr = pResultInfo;
            nRetVal = 0;
          }
        }
        break;

        case REQUEST:
        {
          if(rLocal.addr != 0)
          {
            ResultInfo* pResultInfo = static_cast<ResultInfo*>(rLocal.addr);

            if(pResultInfo != 0)
            {
              if(pResultInfo->m_dX < pResultInfo->m_BoundingBox.MaxD(0) &&
                 pResultInfo->m_dY < pResultInfo->m_BoundingBox.MaxD(1))
              {
                datetime::DateTime instant = pSourceType->getInstant();
                typename SourceTypeProperties::spatial_type* pstype =
                pSourceType->val();

                if(pstype != 0)
                {
                  DestinationType* pDestinationType = new DestinationType(true);

                  if(pDestinationType != 0)
                  {
                    raster2::grid2 grid = pstype->getGrid();
                    double gridLength = grid.getLength();
                    double halfGridLength = gridLength / 2.0;
                    int xDimensionSize = DestinationTypeProperties::
                                         GetXDimensionSize();
                    int yDimensionSize = DestinationTypeProperties::
                                         GetYDimensionSize();
                    bool bHasDefinedValue = false;

                    do
                    {
                      pDestinationType->SetInstant(instant);
                      pDestinationType->SetGrid(pResultInfo->m_dX,
                                                pResultInfo->m_dY,
                                                gridLength);

                      for(int row = 0; row < yDimensionSize; row++)
                      {
                        for(int column = 0; column < xDimensionSize; column++)
                        {
                          double x = pResultInfo->m_dX + column * gridLength +
                                     halfGridLength;
                          double y = pResultInfo->m_dY + row * gridLength +
                                     halfGridLength;

                          typename DestinationTypeProperties::TypeProperties::
                          PropertiesType value = pstype->atlocation(x, y);

                          if(DestinationTypeProperties::TypeProperties::
                             IsUndefinedValue(value) == false)
                          {
                            bHasDefinedValue = true;
                            pDestinationType->SetValue(x, y, value, true);
                          }
                        }
                      }

                      pResultInfo->m_dX += xDimensionSize * gridLength;

                      if(pResultInfo->m_dX >=
                         pResultInfo->m_BoundingBox.MaxD(0))
                      {
                        pResultInfo->m_dY += yDimensionSize * gridLength;

                        if(pResultInfo->m_dY <
                           pResultInfo->m_BoundingBox.MaxD(1))
                        {
                          pResultInfo->m_dX =
                          pResultInfo->m_BoundingBox.MinD(0);
                        }
                        
                        else
                        {
                          break;
                        }
                      }
                    }

                    while(bHasDefinedValue == false);
                    
                    if(bHasDefinedValue == true)
                    {
                      // return the next stream element
                      rResult.addr = pDestinationType;
                      nRetVal = YIELD;
                    }
                    
                    else
                    {
                      delete pDestinationType;
                      pDestinationType = 0;
                      
                      // always set the result to null before return CANCEL
                      rResult.addr = 0;
                      nRetVal = CANCEL;
                    }
                  }

                  delete pstype;
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
            ResultInfo* pResultInfo = static_cast<ResultInfo*>(rLocal.addr);

            if(pResultInfo != 0)
            {
              delete pResultInfo;
              rLocal.addr = 0;
            }
          }

          nRetVal = 0;
        }
        break;

        default:
        {
          assert(false);
        }
        break;
      }
    }
  }

  return nRetVal;
}

/*
definition of tilesFunctions array.

*/

ValueMapping tilesFunctions[] =
{
  tilesFunctiontgrid,
  tilesFunctionmtgrid,
  tilesFunctiont<raster2::sint, tint, tProperties<int> >,
  tilesFunctiont<raster2::sreal, treal, tProperties<double> >,
  tilesFunctiont<raster2::sbool, tbool, tProperties<char> >,
  tilesFunctiont<raster2::sstring, tstring, tProperties<std::string> >,
  tilesFunctionmt<raster2::msint, mtint, mtProperties<int> >,
  tilesFunctionmt<raster2::msreal, mtreal, mtProperties<double> >,
  tilesFunctionmt<raster2::msbool, mtbool, mtProperties<char> >,
  tilesFunctionmt<raster2::msstring, mtstring, mtProperties<std::string> >,
  tilesFunctionit<raster2::isint, raster2::istype_helper<int>,
                  itint, itProperties<int> >,
  tilesFunctionit<raster2::isreal, raster2::istype_helper<double>,
                  itreal, itProperties<double> >,
  tilesFunctionit<raster2::isbool, raster2::istype_helper<char>,
                  itbool, itProperties<char> >,
  tilesFunctionit<raster2::isstring, raster2::istype_helper<std::string>,
                  itstring, itProperties<std::string> >,
  0
};

/*
Method tilesSelectFunction returns the index of specific tiles function
in tilesFunctions array depending on the arguments.

author: Dirk Zacher
parameters: arguments - arguments of tiles operator
return value: index of specific tiles function in tilesFunctions
exceptions: -

*/

int tilesSelectFunction(ListExpr arguments)
{
  int functionIndex = -1;

  if(arguments != 0)
  {
    NList argumentsList(arguments);

    if(argumentsList.hasLength(1))
    {
      NList argument1 = argumentsList.first();
      const int TYPE_NAMES = 14;
      const std::string TYPE_NAMES_ARRAY[TYPE_NAMES] =
      {
        raster2::grid2::BasicType(),
        raster2::grid3::BasicType(),
        raster2::sint::BasicType(),
        raster2::sreal::BasicType(),
        raster2::sbool::BasicType(),
        raster2::sstring::BasicType(),
        raster2::msint::BasicType(),
        raster2::msreal::BasicType(),
        raster2::msbool::BasicType(),
        raster2::msstring::BasicType(),
        raster2::isint::BasicType(),
        raster2::isreal::BasicType(),
        raster2::isbool::BasicType(),
        raster2::isstring::BasicType()
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
Method tilesTypeMappingFunction returns the return value type
of tiles operator in the form of a ListExpr.

author: Dirk Zacher
parameters: arguments - arguments of tiles operator
return value: return value type of tiles operator
exceptions: -

*/

ListExpr tilesTypeMappingFunction(ListExpr arguments)
{
  ListExpr type = NList::typeError("Operator tiles expects "
                                   "a grid2 object, a grid3 object, "
                                   "a s type object, a ms type object or "
                                   "an is type object.");

  if(nl != 0)
  {
    NList argumentsList(arguments);

    if(argumentsList.hasLength(1))
    {
      std::string argument1 = argumentsList.first().str();

      if(argument1 == raster2::grid2::BasicType())
      {
        type = NList(tgrid::BasicType()).listExpr();
      }

      else if(argument1 == raster2::grid3::BasicType())
      {
        type = NList(mtgrid::BasicType()).listExpr();
      }

      else if(IssType(argument1))
      {
        type = nl->TwoElemList(nl->SymbolAtom(Stream<Attribute>::BasicType()),
                               nl->SymbolAtom(GettType(argument1)));
      }

      else if(IsmsType(argument1))
      {
        type = nl->TwoElemList(nl->SymbolAtom(Stream<Attribute>::BasicType()),
                               nl->SymbolAtom(GetmtType(argument1)));
      }

      else if(IsisType(argument1))
      {
        type = nl->TwoElemList(nl->SymbolAtom(Stream<Attribute>::BasicType()),
                               nl->SymbolAtom(GetitType(argument1)));
      }
    }
  }

  return type;
}

}
