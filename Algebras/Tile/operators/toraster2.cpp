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

#include "DateTime.h"

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

#include "toraster2.h"
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
Method toraster2Functiontgrid converts a Tile Algebra tgrid object
to a Raster2 Algebra grid2 object.

author: Dirk Zacher
parameters: pArguments - a pointer to the arguments of toraster2 operator
            rResult - reference to a Word containing the result
            message - message to distinguish call modes of toraster2Functiontgrid
            rLocal - reference to a Word to store local method information
            supplier - an Address to a supplier of information of operator tree
return value: 0 if toraster2Functiontgrid successfully executed, otherwise FAILURE
exceptions: -

*/

int toraster2Functiontgrid(Word* pArguments,
                           Word& rResult,
                           int message,
                           Word& rLocal,
                           Supplier supplier)
{
  int nRetVal = FAILURE;

  if(qp != 0 &&
     pArguments != 0)
  {
    tgrid* pGrid = static_cast<tgrid*>(pArguments[0].addr);

    if(pGrid != 0)
    {
      rResult = qp->ResultStorage(supplier);

      if(rResult.addr != 0)
      {
        raster2::grid2* pResult = static_cast<raster2::grid2*>(rResult.addr);

        if(pResult != 0)
        {
          if(pGrid->IsDefined())
          {
            pResult->set(pGrid->GetX(),
                         pGrid->GetY(),
                         pGrid->GetLength());
            nRetVal = 0;
          }
        }
      }
    }
  }

  return nRetVal;
}

/*
Method toraster2Functionmtgrid converts a Tile Algebra mtgrid object
to a Raster2 Algebra grid3 object.

author: Dirk Zacher
parameters: pArguments - a pointer to the arguments of toraster2 operator
            rResult - reference to a Word containing the result
            message - message to distinguish call modes of toraster2Functionmtgrid
            rLocal - reference to a Word to store local method information
            supplier - an Address to a supplier of information of operator tree
return value: 0 if toraster2Functionmtgrid successfully executed, otherwise FAILURE
exceptions: -

*/

int toraster2Functionmtgrid(Word* pArguments,
                            Word& rResult,
                            int message,
                            Word& rLocal,
                            Supplier supplier)
{
  int nRetVal = FAILURE;

  if(qp != 0 &&
     pArguments != 0)
  {
    mtgrid* pGrid = static_cast<mtgrid*>(pArguments[0].addr);

    if(pGrid != 0)
    {
      rResult = qp->ResultStorage(supplier);

      if(rResult.addr != 0)
      {
        raster2::grid3* pResult = static_cast<raster2::grid3*>(rResult.addr);

        if(pResult != 0)
        {
          if(pGrid->IsDefined())
          {
            pResult->set(pGrid->GetX(),
                         pGrid->GetY(),
                         pGrid->GetLength(),
                         pGrid->GetDuration());
            nRetVal = 0;
          }
        }
      }
    }
  }

  return nRetVal;
}

/*
Template method toraster2Functiont converts a stream of Tile Algebra t type
objects to a Raster2 Algebra s type object.

author: Dirk Zacher
parameters: pArguments - a pointer to the arguments of toraster2 operator
            rResult - reference to a Word containing the result
            message - message to distinguish call modes of toraster2Functiont
            rLocal - reference to a Word to store local method information
            supplier - an Address to a supplier of information of operator tree
return value: 0 if toraster2Functiont successfully executed, otherwise FAILURE
exceptions: -

*/

template <typename SourceType, typename SourceTypeProperties,
          typename DestinationType>
int toraster2Functiont(Word* pArguments,
                       Word& rResult,
                       int message,
                       Word& rLocal,
                       Supplier supplier)
{
  int nRetVal = FAILURE;

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
                double sourceHalfGridLength = sourceGridLength / 2.0;

                if(sourceGridX < grid.getOriginX() &&
                   sourceGridY < grid.getOriginY())
                {
                  // set smallest x and y coordinates
                  grid.set(sourceGridX, sourceGridY, sourceGridLength);
                  pDestinationType->setGrid(grid);
                }

                Index<2> minimumIndex;
                Index<2> maximumIndex;
                bool bOK = pSourceType->GetBoundingBoxIndexes(minimumIndex,
                                                              maximumIndex);

                if(bOK == true)
                {
                  for(int row = minimumIndex[1]; row < maximumIndex[1]; row++)
                  {
                    for(int column = minimumIndex[0]; column < maximumIndex[0];
                        column++)
                    {
                      double x = sourceGridX + column * sourceGridLength +
                                 sourceHalfGridLength;
                      double y = sourceGridY + row * sourceGridLength +
                                 sourceHalfGridLength;
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

        nRetVal = 0;
      }
    }
  }

  return nRetVal;
}

/*
Template method toraster2Functionmt converts a stream of Tile Algebra mt type
objects to a Raster2 Algebra ms type object.

author: Dirk Zacher
parameters: pArguments - a pointer to the arguments of toraster2 operator
            rResult - reference to a Word containing the result
            message - message to distinguish call modes of toraster2Functionmt
            rLocal - reference to a Word to store local method information
            supplier - an Address to a supplier of information of operator tree
return value: 0 if toraster2Functionmt successfully executed, otherwise FAILURE
exceptions: -

*/

template <typename SourceType, typename SourceTypeProperties,
          typename DestinationType>
int toraster2Functionmt(Word* pArguments,
                        Word& rResult,
                        int message,
                        Word& rLocal,
                        Supplier supplier)
{
  int nRetVal = FAILURE;

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
                double sourceHalfGridLength = sourceGridLength / 2.0;
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

                Index<3> minimumIndex;
                Index<3> maximumIndex;
                bool bOK = pSourceType->GetBoundingBoxIndexes(minimumIndex,
                                                              maximumIndex);

                if(bOK == true)
                {
                  for(int time = minimumIndex[2]; time < maximumIndex[2];
                      time++)
                  {
                    for(int row = minimumIndex[1]; row < maximumIndex[1];
                        row++)
                    {
                      for(int column = minimumIndex[0];
                          column < maximumIndex[0]; column++)
                      {
                        double x = sourceGridX + column * sourceGridLength +
                                   sourceHalfGridLength;
                        double y = sourceGridY + row * sourceGridLength +
                                   sourceHalfGridLength;
                        double t = time * sourceGridDuration.ToDouble() +
                                   sourceGridDuration.ToDouble() / 2.0;
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

        nRetVal = 0;
      }
    }
  }

  return nRetVal;
}

/*
Template method toraster2Functionit converts a stream of Tile Algebra it type
objects to a Raster2 Algebra is type object.

author: Dirk Zacher
parameters: pArguments - a pointer to the arguments of toraster2 operator
            rResult - reference to a Word containing the result
            message - message to distinguish call modes of toraster2Functionit
            rLocal - reference to a Word to store local method information
            supplier - an Address to a supplier of information of operator tree
return value: 0 if toraster2Functionit successfully executed, otherwise FAILURE
exceptions: -

*/

template <typename SourceType, typename SourceTypeProperties,
          typename DestinationType, typename DestinationTypeProperties>
int toraster2Functionit(Word* pArguments,
                        Word& rResult,
                        int message,
                        Word& rLocal,
                        Supplier supplier)
{
  int nRetVal = FAILURE;

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
            Instant* pInstant = new Instant(0.0);
            typename DestinationTypeProperties::spatial_type* pstype =
            new typename DestinationTypeProperties::spatial_type();

            if(pInstant != 0 &&
               pstype != 0)
            {
              qp->Open(pArguments[0].addr);
              
              raster2::grid2 grid(std::numeric_limits<double>::max(),
                                  std::numeric_limits<double>::max(),
                                  std::numeric_limits<double>::max());
              Word word;
              qp->Request(pArguments[0].addr, word);

              while(qp->Received(pArguments[0].addr))
              {
                SourceType* pSourceType = static_cast<SourceType*>(word.addr);

                if(pSourceType != 0)
                {
                  pDestinationType->setDefined(true);
                  pstype->setDefined(true);

                  pSourceType->inst(*pInstant);
                  pDestinationType->setInstant(pInstant);
                  pDestinationType->setValues(pstype);

                  tgrid sourceGrid;
                  pSourceType->getgrid(sourceGrid);

                  double sourceGridX = sourceGrid.GetX();
                  double sourceGridY = sourceGrid.GetY();
                  double sourceGridLength = sourceGrid.GetLength();
                  double sourceHalfGridLength = sourceGridLength / 2.0;

                  if(sourceGridX < grid.getOriginX() &&
                     sourceGridY < grid.getOriginY())
                  {
                    // set smallest x and y coordinates
                    grid.set(sourceGridX, sourceGridY, sourceGridLength);
                    pstype->setGrid(grid);
                  }

                  Index<2> minimumIndex;
                  Index<2> maximumIndex;
                  bool bOK = pSourceType->GetBoundingBoxIndexes(minimumIndex,
                                                                maximumIndex);

                  if(bOK == true)
                  {
                    for(int row = minimumIndex[1]; row < maximumIndex[1];
                        row++)
                    {
                      for(int column = minimumIndex[0];
                          column < maximumIndex[0]; column++)
                      {
                        double x = sourceGridX + column * sourceGridLength +
                                   sourceHalfGridLength;
                        double y = sourceGridY + row * sourceGridLength +
                                   sourceHalfGridLength;
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
                            pstype->setatlocation(x, y, value);
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

              if(pDestinationType->isDefined() == false)
              {
                delete pInstant;
                pInstant = 0;

                delete pstype;
                pstype = 0;
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

        nRetVal = 0;
      }
    }
  }

  return nRetVal;
}

/*
definition of toraster2Functions array.

*/

ValueMapping toraster2Functions[] =
{
  toraster2Functiontgrid,
  toraster2Functionmtgrid,
  toraster2Functiont<tint, tProperties<int>, raster2::sint>,
  toraster2Functiont<treal, tProperties<double>, raster2::sreal>,
  toraster2Functiont<tbool, tProperties<char>, raster2::sbool>,
  toraster2Functiont<tstring, tProperties<std::string>, raster2::sstring>,
  toraster2Functionmt<mtint, mtProperties<int>, raster2::msint>,
  toraster2Functionmt<mtreal, mtProperties<double>, raster2::msreal>,
  toraster2Functionmt<mtbool, mtProperties<char>, raster2::msbool>,
  toraster2Functionmt<mtstring, mtProperties<std::string>, raster2::msstring>,
  toraster2Functionit<itint, itProperties<int>,
                      raster2::isint, raster2::istype_helper<int> >,
  toraster2Functionit<itreal, itProperties<double>,
                      raster2::isreal, raster2::istype_helper<double> >,
  toraster2Functionit<itbool, itProperties<char>,
                      raster2::isbool, raster2::istype_helper<char> >,
  toraster2Functionit<itstring, itProperties<std::string>,
                      raster2::isstring, raster2::istype_helper<std::string> >,
  0
};

/*
Method toraster2SelectFunction returns the index of specific toraster2 function
in toraster2Functions array depending on the arguments.

author: Dirk Zacher
parameters: arguments - arguments of toraster2 operator
return value: index of specific toraster2 function in toraster2Functions
exceptions: -

*/

int toraster2SelectFunction(ListExpr arguments)
{
  int functionIndex = -1;

  if(arguments != 0)
  {
    NList argumentsList(arguments);

    if(argumentsList.hasLength(1))
    {
      NList argument1 = argumentsList.first();

      if(argument1.hasLength(2))
      {
        argument1 = argument1.second();
      }

      const int TYPE_NAMES = 14;
      const std::string TYPE_NAMES_ARRAY[TYPE_NAMES] =
      {
        tgrid::BasicType(),
        mtgrid::BasicType(),
        tint::BasicType(),
        treal::BasicType(),
        tbool::BasicType(),
        tstring::BasicType(),
        mtint::BasicType(),
        mtreal::BasicType(),
        mtbool::BasicType(),
        mtstring::BasicType(),
        itint::BasicType(),
        itreal::BasicType(),
        itbool::BasicType(),
        itstring::BasicType()
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
Method toraster2TypeMappingFunction returns the return value type
of toraster2 operator in the form of a ListExpr.

author: Dirk Zacher
parameters: arguments - arguments of toraster2 operator
return value: return value type of toraster2 operator
exceptions: -

*/

ListExpr toraster2TypeMappingFunction(ListExpr arguments)
{
  ListExpr type = NList::typeError("Operator toraster2 expects "
                                   "a tgrid object, a mtgrid object, "
                                   "a stream of t type objects, "
                                   "a stream of mt type objects or "
                                   "a stream of it type objects.");

  if(nl != 0)
  {
    NList argumentsList(arguments);

    if(argumentsList.hasLength(1))
    {
      NList argument1List = argumentsList.first();

      if(argument1List.hasStringValue())
      {
        std::string argument1 = argument1List.str();

        if(argument1 == tgrid::BasicType())
        {
          type = NList(raster2::grid2::BasicType()).listExpr();
        }

        else if(argument1 == mtgrid::BasicType())
        {
          type = NList(raster2::grid3::BasicType()).listExpr();
        }
      }

      else if(Stream<Attribute>::checkType(argument1List.listExpr()))
      {
        std::string typeName = argument1List.second().str();

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

          else if(IsitType(typeName))
          {
            type = NList(GetisType(typeName)).listExpr();
          }
        }
      }
    }
  }

  return type;
}

}
