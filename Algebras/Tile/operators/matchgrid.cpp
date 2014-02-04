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

#include "RelationAlgebra.h"

/*
TileAlgebra includes

*/

#include "matchgrid.h"
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
Template method matchgridFunctiont implements the matchgrid operator
functionality for t datatypes.

author: Dirk Zacher
parameters: pArguments - a pointer to the arguments of matchgrid operator
            rResult - reference to a Word containing the result
            message - message to distinguish call modes of matchgridFunctiont
            rLocal - reference to a Word to store local method information
            supplier - an Address to a supplier of information of operator tree
return value: 0 if matchgridFunctiont successfully executed, otherwise FAILURE
exceptions: -

*/

template <typename SourceType,
          typename SourceTypeProperties,
          typename DestinationType,
          typename DestinationTypeProperties,
          typename Traits>
int matchgridFunctiont(Word* pArguments,
                       Word& rResult,
                       int message,
                       Word& rLocal,
                       Supplier supplier)
{
  int nRetVal = FAILURE;

  if(qp != 0 &&
     pArguments != 0)
  {
    SourceType* pSourceType = static_cast<SourceType*>
                              (pArguments[0].addr);
    tgrid* pGrid = static_cast<tgrid*>
                   (pArguments[1].addr);

    if(pSourceType != 0 &&
       pGrid != 0)
    {
      rResult = qp->ResultStorage(supplier);

      if(rResult.addr != 0)
      {
        DestinationType* pResult = static_cast<DestinationType*>
                                   (rResult.addr);

        if(pResult != 0)
        {
          pResult->SetDefined(false);

          if(pSourceType->IsDefined() &&
             pGrid->IsDefined())
          {
            pResult->SetDefined(true);
            pResult->SetGrid(*pGrid);

            Address pFunction = pArguments[2].addr;
            CcBool* pUseWeight = static_cast<CcBool*>
                                 (pArguments[3].addr);
            CcInt* pAttributeAlgebraID =
            static_cast<CcInt*>(pArguments[5].addr);
            CcInt* pAttributeTypeID = static_cast<CcInt*>
                                      (pArguments[6].addr);

            if(pFunction!= 0 &&
               pUseWeight != 0 &&
               pAttributeAlgebraID != 0 &&
               pAttributeTypeID != 0)
            {
              // create tuple type for temporary relation
              ListExpr tupleType = NList(
                                   NList(NList("tuple"),
                                         NList(NList("Elem"),
                                         NList(NList(
                                               pAttributeAlgebraID->
                                               GetIntval()),
                                               NList(
                                               pAttributeTypeID->
                                               GetIntval()))).
                                               enclose()
                                        )
                                        ).listExpr();

              TupleType* pTupleType = new TupleType(tupleType);

              if(pTupleType != 0)
              {
                tgrid sourceGrid;
                pSourceType->getgrid(sourceGrid);

                Rectangle<2> sourceBoundingBox;
                pSourceType->bbox(sourceBoundingBox);

                Index<2> resultStartIndex =
                pResult->GetLocationIndex(sourceBoundingBox.MinD(0),
                                          sourceBoundingBox.MinD(1));

                Index<2> resultEndIndex = pResult->GetLocationIndex(
                                          sourceBoundingBox.MaxD(0),
                                          sourceBoundingBox.MaxD(1));

                // create temporary relation
                TupleBuffer rel;

                ArgVector& argumentsVector =
                *qp->Argument(pFunction);
                argumentsVector[0].setAddr(&rel);
                
                // compute area of a single cell in source for weight
                double sourceArea = pow(sourceGrid.GetLength(), 2);

                double gridX = pGrid->GetX();
                double gridY = pGrid->GetY();
                double gridLength = pGrid->GetLength();
                double sourceGridX = sourceGrid.GetX();
                double sourceGridY = sourceGrid.GetY();
                double sourceGridLength = sourceGrid.GetLength();

                for(int resultRow = resultStartIndex[1];
                    resultRow <= resultEndIndex[1];
                    resultRow++)
                {
                  for(int resultColumn = resultStartIndex[0];
                      resultColumn <= resultEndIndex[0];
                      resultColumn++)
                  { 
                    Index<2> resultCurrentIndex
                    ((int[]){resultColumn, resultRow});
                    
                    Rectangle<2> currentBoundingBox
                    (true,
                     gridX +
                     resultCurrentIndex[0] * gridLength,
                     gridX +
                     (resultCurrentIndex[0] + 1) * gridLength,
                     gridY +
                     resultCurrentIndex[1] * gridLength,
                     gridY +
                     (resultCurrentIndex[1] + 1) * gridLength);

                    Index<2> sourceStartIndex
                    ((int[]){(int)std::floor((currentBoundingBox.MinD(0) -
                                         sourceGridX) /
                                         sourceGridLength),
                             (int)std::floor((currentBoundingBox.MinD(1) -
                                         sourceGridY) /
                                         sourceGridLength)});
                    Index<2> sourceEndIndex
                    ((int[]){(int)std::floor((currentBoundingBox.MaxD(0) -
                                         sourceGridX) /
                                         sourceGridLength),
                             (int)std::floor((currentBoundingBox.MaxD(1) -
                                         sourceGridY) /
                                         sourceGridLength)});
                    rel.Clear();

                    for(int sourceRow = sourceStartIndex[1];
                        sourceRow <= sourceEndIndex[1];
                        sourceRow++)
                    {
                      for(int sourceColumn = sourceStartIndex[0];
                          sourceColumn <= sourceEndIndex[0];
                          sourceColumn++)
                      {
                        Index<2> sourceCurrentIndex
                        ((int[]){sourceColumn, sourceRow});
                        
                        typename SourceTypeProperties::
                        TypeProperties::PropertiesType value =
                        pSourceType->GetValue(sourceCurrentIndex);

                        if(SourceTypeProperties::TypeProperties::
                           IsUndefinedValue(value) == false)
                        {
                          // weight if required
                          if(Traits::m_bCanWeight == true &&
                             pUseWeight->IsDefined() &&
                             pUseWeight->GetValue())
                          {
                            Rectangle<2> sourceBoundingBox
                            (sourceGridX +
                             sourceCurrentIndex[0] *
                             sourceGridLength,
                             sourceGridX +
                             (sourceCurrentIndex[0] + 1) *
                             sourceGridLength,
                             sourceGridY +
                             sourceCurrentIndex[1] *
                             sourceGridLength,
                             sourceGridY +
                             (sourceCurrentIndex[1] + 1) *
                             sourceGridLength);
                              
                            Rectangle<2> overlapRectangle =
                            currentBoundingBox.Intersection
                            (sourceBoundingBox);

                            Traits::Weight(value,
                                           overlapRectangle.Area() /
                                           sourceArea);
                          }

                          Tuple* pTuple = new Tuple(pTupleType);

                          if(pTuple != 0)
                          {
                            typename SourceTypeProperties::
                            TypeProperties::WrapperType*
                            pWrappedValue =
                            new typename SourceTypeProperties::
                            TypeProperties::WrapperType
                            (SourceTypeProperties::TypeProperties::
                            GetWrappedValue(value));

                            if(pWrappedValue != 0)
                            {
                              pTuple->PutAttribute(0, pWrappedValue);
                              rel.AppendTuple(pTuple);
                            }

                            pTuple->DeleteIfAllowed();
                          }
                        }
                      }
                    }

                    // evaluate function
                    Word word;
                    qp->Request(pFunction, word);

                    if(word.addr != 0)
                    {
                      typename DestinationTypeProperties::
                      TypeProperties::WrapperType* pWrappedValue =
                      static_cast<typename
                      DestinationTypeProperties::TypeProperties::
                      WrapperType*>(word.addr);

                      if(pWrappedValue != 0 &&
                         pWrappedValue->IsDefined())
                      {
                        typename DestinationTypeProperties::
                        TypeProperties::PropertiesType value =
                        DestinationTypeProperties::TypeProperties::
                        GetUnwrappedValue(*pWrappedValue);

                        if(DestinationTypeProperties::
                           TypeProperties::IsUndefinedValue(value) ==
                           false)
                        {
                          pResult->SetValue(resultCurrentIndex,
                                            value, true);
                        }
                      }
                    }
                  }
                }

                 pTupleType->DeleteIfAllowed();
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
Template method matchgridFunctionmt implements
the matchgrid operator functionality
for mt datatypes.

author: Dirk Zacher
parameters: pArguments - a pointer to the arguments of matchgrid operator
            rResult - reference to a Word containing the result
            message - message to distinguish call modes
                      of matchgridFunctionmt
            rLocal - reference to a Word to store local method information
            supplier - an Address to a supplier of information
                       of operator tree
return value: 0 if matchgridFunctionmt successfully executed,
              otherwise FAILURE
exceptions: -

*/

template <typename SourceType,
          typename SourceTypeProperties,
          typename DestinationType,
          typename DestinationTypeProperties,
          typename Traits>
int matchgridFunctionmt(Word* pArguments,
                        Word& rResult,
                        int message,
                        Word& rLocal,
                        Supplier supplier)
{
  int nRetVal = FAILURE;

  if(qp != 0 &&
     pArguments != 0)
  {
    SourceType* pSourceType = static_cast<SourceType*>
                              (pArguments[0].addr);
    mtgrid* pGrid = static_cast<mtgrid*>
                    (pArguments[1].addr);

    if(pSourceType != 0 &&
       pGrid != 0)
    {
      rResult = qp->ResultStorage(supplier);

      if(rResult.addr != 0)
      {
        DestinationType* pResult = static_cast<DestinationType*>
                                   (rResult.addr);

        if(pResult != 0)
        {
          pResult->SetDefined(false);

          if(pSourceType->IsDefined() &&
             pGrid->IsDefined())
          {
            pResult->SetDefined(true);
            pResult->SetGrid(*pGrid);

            Address pFunction = pArguments[2].addr;
            CcBool* pUseWeight = static_cast<CcBool*>
                                 (pArguments[3].addr);
            CcString* pAttributeName =
            static_cast<CcString*>(pArguments[4].addr);
            CcInt* pAttributeAlgebraID =
            static_cast<CcInt*>(pArguments[5].addr);
            CcInt* pAttributeTypeID = static_cast<CcInt*>
                                      (pArguments[6].addr);

            if(pFunction!= 0 &&
               pUseWeight != 0 &&
               pAttributeAlgebraID != 0 &&
               pAttributeTypeID != 0)
            {
              if(pUseWeight->IsDefined() == false)
              {
                pUseWeight->Set(true, false);
              }
              
              ListExpr relationDescription = NList(
                                             NList("rel"),
                                             NList(NList("tuple"),
                                             NList(
                                             NList(
                                             pAttributeName->
                                             toText()),
                                             NList(
                                             NList(
                                             pAttributeAlgebraID->
                                             GetIntval()),
                                             NList(
                                             pAttributeTypeID->
                                             GetIntval()))
                                             ).enclose()
                                             )).listExpr();

              ListExpr tupleType = NList(relationDescription).
                                   second().listExpr();

              mtgrid sourceGrid;
              pSourceType->getgrid(sourceGrid);

              Rectangle<3> sourceBoundingBox;
              pSourceType->bbox(sourceBoundingBox);
              
              Index<3> resultStartIndex =
              pResult->GetLocationIndex(sourceBoundingBox.MinD(0),
                                        sourceBoundingBox.MinD(1),
                                        sourceBoundingBox.MinD(2));
              Index<3> resultEndIndex =
              pResult->GetLocationIndex(sourceBoundingBox.MaxD(0),
                                        sourceBoundingBox.MaxD(1),
                                        sourceBoundingBox.MaxD(2));

              Relation rel(relationDescription, true);

              ArgVector& argumentsVector = *qp->Argument(pFunction);
              
              double sourceArea = pow(sourceGrid.GetLength(), 2) *
                                      sourceGrid.GetDuration().
                                      ToDouble();
              
              double gridX = pGrid->GetX();
              double gridY = pGrid->GetY();
              double gridLength = pGrid->GetLength();
              double gridDuration = pGrid->GetDuration().ToDouble();
              double sourceGridX = sourceGrid.GetX();
              double sourceGridY = sourceGrid.GetY();
              double sourceGridLength = sourceGrid.GetLength();
              double sourceGridDuration = sourceGrid.GetDuration().
                                          ToDouble();

              for(int resultTime = resultStartIndex[2];
                  resultTime <= resultEndIndex[2];
                  resultTime++)
              {
                for(int resultRow = resultStartIndex[1];
                      resultRow <= resultEndIndex[1];
                      resultRow++)
                {
                  for(int resultColumn = resultStartIndex[0];
                      resultColumn <= resultEndIndex[0];
                      resultColumn++)
                  {
                    Index<3> resultCurrentIndex
                    ((int[]){resultColumn, resultRow, resultTime});

                    Rectangle<3> currentBoundingBox
                    (true,
                     gridX +
                     resultCurrentIndex[0] * gridLength,
                     gridX +
                     (resultCurrentIndex[0] + 1) * gridLength,
                     gridY +
                     resultCurrentIndex[1] * gridLength,
                     gridY +
                     (resultCurrentIndex[1] + 1) * gridLength,
                     resultCurrentIndex[2] * gridDuration,
                     (resultCurrentIndex[2] + 1) * gridDuration);

                    Index<3> sourceStartIndex
                    ((int[]){(int)std::floor((currentBoundingBox.MinD(0) -
                                         sourceGridX) /
                                         sourceGridLength),
                             (int)std::floor((currentBoundingBox.MinD(1) -
                                         sourceGridY) /
                                         sourceGridLength),
                             (int)std::floor((currentBoundingBox.MinD(2) /
                                         sourceGridDuration))});
                    Index<3> sourceEndIndex
                    ((int[]){(int)std::floor((currentBoundingBox.MaxD(0) -
                                         sourceGridX) /
                                         sourceGridLength),
                             (int)std::floor((currentBoundingBox.MaxD(1) -
                                         sourceGridY) /
                                         sourceGridLength),
                             (int)std::floor((currentBoundingBox.MaxD(2) /
                                         sourceGridDuration))});
                    rel.Clear();

                    for(int sourceTime = sourceStartIndex[2];
                        sourceTime <= sourceEndIndex[2];
                        sourceTime++)
                    {
                      for(int sourceRow = sourceStartIndex[1];
                          sourceRow <= sourceEndIndex[1];
                          sourceRow++)
                      {
                        for(int sourceColumn = sourceStartIndex[0];
                            sourceColumn <= sourceEndIndex[0];
                            sourceColumn++)
                        {
                          Index<3> sourceCurrentIndex
                          ((int[]){sourceColumn, sourceRow, sourceTime});

                          typename SourceTypeProperties::
                          TypeProperties::PropertiesType value =
                          pSourceType->GetValue(sourceCurrentIndex);

                          if(SourceTypeProperties::TypeProperties::
                             IsUndefinedValue(value) == false)
                          {
                            // weight if required
                            if(Traits::m_bCanWeight == true &&
                               pUseWeight->IsDefined() &&
                               pUseWeight->GetValue())
                            {
                              Rectangle<3> sourceBoundingBox
                              (sourceGridX +
                               sourceCurrentIndex[0] *
                               sourceGridLength,
                               sourceGridX +
                               (sourceCurrentIndex[0] + 1) *
                               sourceGridLength,
                               sourceGridY +
                               sourceCurrentIndex[1] *
                               sourceGridLength,
                               sourceGridY +
                               (sourceCurrentIndex[1] + 1) *
                               sourceGridLength,
                               sourceCurrentIndex[2] *
                               sourceGridDuration,
                               (sourceCurrentIndex[2] + 1) *
                               sourceGridDuration);

                              Rectangle<3> overlapRectangle =
                              currentBoundingBox.Intersection
                              (sourceBoundingBox);

                              Traits::Weight(value,
                                             overlapRectangle.
                                             Area() / sourceArea);
                            }

                            Tuple* pTuple = new Tuple(tupleType);

                            if(pTuple != 0)
                            {
                              typename SourceTypeProperties::
                              TypeProperties::WrapperType*
                              pWrappedValue =
                              new typename SourceTypeProperties::
                              TypeProperties::WrapperType
                              (SourceTypeProperties::TypeProperties::
                              GetWrappedValue(value));

                              if(pWrappedValue != 0)
                              {
                                pTuple->PutAttribute(0,
                                                     pWrappedValue);
                                rel.AppendTuple(pTuple);
                              }

                              pTuple->DeleteIfAllowed();
                            }
                          }
                        }
                      }
                    }

                    argumentsVector[0].setAddr(&rel);
                    Word word;
                    qp->Request(pFunction, word);

                    if(word.addr != 0)
                    {
                      typename DestinationTypeProperties::
                      TypeProperties::WrapperType* pWrappedValue =
                      static_cast<typename
                      DestinationTypeProperties::TypeProperties::
                      WrapperType*>(word.addr);

                      if(pWrappedValue != 0 &&
                         pWrappedValue->IsDefined())
                      {
                        typename DestinationTypeProperties::
                        TypeProperties::PropertiesType value =
                        DestinationTypeProperties::TypeProperties::
                        GetUnwrappedValue(*pWrappedValue);

                        if(DestinationTypeProperties::
                           TypeProperties::IsUndefinedValue(value) ==
                           false)
                        {
                          pResult->SetValue(resultCurrentIndex,
                                            value, true);
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
Template struct matchgridTraits represents traits
of a matchgrid operator datatype.

author: Dirk Zacher

*/

template <typename Type>
struct matchgridTraits
{
  /*
  Member m_bCanWeight indicates if matchgrid operator datatype values
  can be weighted.

  */

  static const bool m_bCanWeight;

  /*
  Method Weight implements weight mechanism
  for matchgrid operator datatypes.

  author: Dirk Zacher
  parameters: rType - reference to a matchgrid operator datatype
              rWeightFactor - reference to weight factor
  return value: -
  exceptions: -

  */

  static inline void Weight(Type& rType,
                            const double& rWeightFactor)
  {

  };
};

/*
Member of matchgridTraits<Type> is initialized false.

*/

template <typename Type>
const bool matchgridTraits<Type>::m_bCanWeight = false;

/*
Template struct matchgridTraits<int> represents traits of datatype int.

author: Dirk Zacher

*/

template <>
struct matchgridTraits<int>
{
  /*
  Member m_bCanWeight indicates if datatype int values
  can be weighted.

  */

  static const bool m_bCanWeight;

  /*
  Method Weight implements weight mechanism for datatype int.

  author: Dirk Zacher
  parameters: rint - reference to an int value
              rWeightFactor - reference to weight factor
  return value: -
  exceptions: -

  */

  static inline void Weight(int& rint,
                            const double& rWeightFactor)
  {
    rint *= rWeightFactor;
  }
};

/*
Member of matchgridTraits<int> is initialized true.

*/

const bool matchgridTraits<int>::m_bCanWeight = true;

/*
Template struct matchgridTraits<double> represents traits of datatype double.

author: Dirk Zacher

*/

template <>
struct matchgridTraits<double>
{
  /*
  Member m_bCanWeight indicates if datatype double values can be weighted.

  */

  static const bool m_bCanWeight;

  /*
  Method Weight implements weight mechanism for datatype double.

  author: Dirk Zacher
  parameters: rdouble - reference to a double value
              rWeightFactor - reference to weight factor
  return value: -
  exceptions: -

  */

  static void Weight(double& rdouble,
                     const double& rWeightFactor)
  {
    rdouble *= rWeightFactor;
  };
};

/*
Member of matchgridTraits<double> is initialized true.

*/

const bool matchgridTraits<double>::m_bCanWeight = true;

/*
definition of matchgridFunctions array.

*/

ValueMapping matchgridFunctions[] =
{
  matchgridFunctiont<tint, tProperties<int>,
                     tint, tProperties<int>,
                     matchgridTraits<int> >,
  matchgridFunctiont<tint, tProperties<int>,
                     treal, tProperties<double>,
                     matchgridTraits<int> >,
  matchgridFunctiont<tint, tProperties<int>,
                     tbool, tProperties<char>,
                     matchgridTraits<int> >,
  matchgridFunctiont<tint, tProperties<int>,
                     tstring, tProperties<std::string>,
                     matchgridTraits<int> >,

  matchgridFunctiont<treal, tProperties<double>,
                     tint, tProperties<int>,
                     matchgridTraits<double> >,
  matchgridFunctiont<treal, tProperties<double>,
                     treal, tProperties<double>,
                     matchgridTraits<double> >,
  matchgridFunctiont<treal, tProperties<double>,
                     tbool, tProperties<char>,
                     matchgridTraits<double> >,
  matchgridFunctiont<treal, tProperties<double>,
                     tstring, tProperties<std::string>,
                     matchgridTraits<double> >,

  matchgridFunctiont<tbool, tProperties<char>,
                     tint, tProperties<int>,
                     matchgridTraits<char> >,
  matchgridFunctiont<tbool, tProperties<char>,
                     treal, tProperties<double>,
                     matchgridTraits<char> >,
  matchgridFunctiont<tbool, tProperties<char>,
                     tbool, tProperties<char>,
                     matchgridTraits<char> >,
  matchgridFunctiont<tbool, tProperties<char>,
                     tstring, tProperties<std::string>,
                     matchgridTraits<char> >,

  matchgridFunctiont<tstring, tProperties<std::string>,
                     tint, tProperties<int>,
                     matchgridTraits<std::string> >,
  matchgridFunctiont<tstring, tProperties<std::string>,
                     treal, tProperties<double>,
                     matchgridTraits<std::string> >,
  matchgridFunctiont<tstring, tProperties<std::string>,
                     tbool, tProperties<char>,
                     matchgridTraits<std::string> >,
  matchgridFunctiont<tstring, tProperties<std::string>,
                     tstring, tProperties<std::string>,
                     matchgridTraits<std::string> >,

  matchgridFunctionmt<mtint, mtProperties<int>,
                      mtint, mtProperties<int>,
                      matchgridTraits<int> >,
  matchgridFunctionmt<mtint, mtProperties<int>,
                      mtreal, mtProperties<double>,
                      matchgridTraits<int> >,
  matchgridFunctionmt<mtint, mtProperties<int>,
                      mtbool, mtProperties<char>,
                      matchgridTraits<int> >,
  matchgridFunctionmt<mtint, mtProperties<int>,
                      mtstring, mtProperties<std::string>,
                      matchgridTraits<int> >,

  matchgridFunctionmt<mtreal, mtProperties<double>,
                      mtint, mtProperties<int>,
                      matchgridTraits<double> >,
  matchgridFunctionmt<mtreal, mtProperties<double>,
                      mtreal, mtProperties<double>,
                      matchgridTraits<double> >,
  matchgridFunctionmt<mtreal, mtProperties<double>,
                      mtbool, mtProperties<char>,
                      matchgridTraits<double> >,
  matchgridFunctionmt<mtreal, mtProperties<double>,
                      mtstring, mtProperties<std::string>,
                      matchgridTraits<double> >,

  matchgridFunctionmt<mtbool, mtProperties<char>,
                      mtint, mtProperties<int>,
                      matchgridTraits<char> >,
  matchgridFunctionmt<mtbool, mtProperties<char>,
                      mtreal, mtProperties<double>,
                      matchgridTraits<char> >,
  matchgridFunctionmt<mtbool, mtProperties<char>,
                      mtbool, mtProperties<char>,
                      matchgridTraits<char> >,
  matchgridFunctionmt<mtbool, mtProperties<char>,
                      mtstring, mtProperties<std::string>,
                      matchgridTraits<char> >,

  matchgridFunctionmt<mtstring, mtProperties<std::string>,
                      mtint, mtProperties<int>,
                      matchgridTraits<std::string> >,
  matchgridFunctionmt<mtstring, mtProperties<std::string>,
                      mtreal, mtProperties<double>,
                      matchgridTraits<std::string> >,
  matchgridFunctionmt<mtstring, mtProperties<std::string>,
                      mtbool, mtProperties<char>,
                      matchgridTraits<std::string> >,
  matchgridFunctionmt<mtstring, mtProperties<std::string>,
                      mtstring, mtProperties<std::string>,
                      matchgridTraits<std::string> >,
  0
};

/*
Method matchgridSelectFunction returns the index of specific matchgrid function
in matchgridFunctions array depending on the arguments.

author: Dirk Zacher
parameters: arguments - arguments of matchgrid operator
return value: index of specific matchgrid function in matchgridFunctions
exceptions: -

*/

int matchgridSelectFunction(ListExpr arguments)
{
  int functionIndex = -1;

  if(arguments != 0)
  {
    NList argumentsList(arguments);

    if(argumentsList.hasLength(4))
    {
      NList argument1 = argumentsList.first();
      std::string argument3 = argumentsList.third().third().str();

      int argument1Index = -1;
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

        if(argument3 == TYPE_NAMES_ARRAY[i])
        {
          argument3Index = i;
        }
      }

      if(argument1Index >= 0 &&
         argument3Index >= 0)
      {
        functionIndex = (argument1Index * 4) +
                        (argument3Index % 4);
      }
    }
  }

  return functionIndex;
}

/*
Method matchgridTypeMappingFunction returns the return value type
of matchgrid operator in the form of a ListExpr.

author: Dirk Zacher
parameters: arguments - arguments of matchgrid operator
return value: return value type of matchgrid operator
exceptions: -

*/

ListExpr matchgridTypeMappingFunction(ListExpr arguments)
{
  ListExpr type = NList::typeError("Operator matchgrid expects "
                                   "a t type object or "
                                   "a mt type object, "
                                   "a tgrid object or "
                                   "a mtgrid object, "
                                   "a relation and a bool object");

  NList argumentsList(arguments);

  if(argumentsList.hasLength(3) ||
     argumentsList.hasLength(4))
  {
    std::string argument1 = argumentsList.first().str();
    bool bIstType = IstType(argument1);
    bool bIsmtType = IsmtType(argument1);

    if(bIstType ||
       bIsmtType)
    {
      std::string argument2 = argumentsList.second().str();
      bool bValidArgument2 = false;

      if(bIstType)
      {
        if(argument2 == tgrid::BasicType())
        {
          bValidArgument2 = true;
        }

        else
        {
          type = NList::typeError("Operator matchgrid expects "
                                  "a tgrid object "
                                  "as second argument.");
        }
      }

      if(bIsmtType)
      {
        if(argument2 == mtgrid::BasicType())
        {
          bValidArgument2 = true;
        }

        else
        {
          type = NList::typeError("Operator matchgrid expects "
                                  "a mtgrid object "
                                  "as second argument.");
        }
      }

      if(bValidArgument2 == true)
      {
        NList argument3 = argumentsList.third();

        if(listutils::isMap<1>(argument3.listExpr()))
        {
          if(listutils::isRelDescription(argument3.second().
                                         listExpr()))
          {
            NList attributeList = argument3.second().second().
                                  second();

            if(attributeList.length() == 1)
            {
              NList attributeTypeList = attributeList.first().
                                        second();
              std::string attributeType = attributeTypeList.str();
              std::string valueWrapperType = GetValueWrapperType
                                             (argument1);

              if(attributeType == valueWrapperType)
              {
                SecondoCatalog* pSecondoCatalog = SecondoSystem::
                                                  GetCatalog();

                if(pSecondoCatalog != 0)
                {
                  int attributeAlgebraID = -1;
                  int attributeTypeID = -1;
                  pSecondoCatalog->LookUpTypeExpr(attributeTypeList.
                                                  listExpr(),
                                                  attributeType,
                                                  attributeAlgebraID,
                                                  attributeTypeID);

                  if(attributeType.empty() == false)
                  {
                    std::string functionResult = argument3.third().
                                                 str();
                    std::string typeName;

                    if(bIstType)
                    {
                      typeName = GettType(functionResult);
                    }

                    else
                    {
                      typeName = GetmtType(functionResult);
                    }

                    if(typeName.empty() == false)
                    {
                      if(argumentsList.hasLength(3))
                      {
                        type = NList(NList(Symbol::APPEND()),
                                     NList(NList(false, false),
                                     NList("", true),
                                     NList(attributeAlgebraID),
                                     NList(attributeTypeID)),
                                     NList(typeName)).listExpr();
                      }
                      
                      else
                      {
                        std::string argument4 = argumentsList.
                                                fourth().str();

                        if(argument4 == CcBool::BasicType())
                        {
                          type = NList(NList(Symbol::APPEND()),
                                       NList(
                                       NList("", true),
                                       NList(attributeAlgebraID),
                                       NList(attributeTypeID)),
                                       NList(typeName)).listExpr();
                        }

                        else
                        {
                          type = NList::typeError("Operator "
                                                  "matchgrid "
                                                  "expects "
                                                  "a bool object "
                                                  "as fourth "
                                                  "argument.");
                        }
                      }
                    }

                    else
                    {
                      type = NList::typeError("Cannot find typename "
                                              "to store.");
                    }
                  }

                  else
                  {
                    type = NList::typeError("Cannot find "
                                            "attribute type " +
                                            attributeTypeList.
                                            convertToString());
                  }
                }
              }

              else
              {
                type = NList::typeError("Attribute must be "
                                        "of type " +
                                        valueWrapperType);
              }
            }

            else
            {
              type = NList::typeError("Relation must have exactly "
                                      "one attribute.");
            }
          }

          else
          {
            type = NList::typeError("Argument function must accept "
                                    "a relation as parameter.");
          }
        }

        else
        {
          type = NList::typeError("Third argument must be "
                                  "a function of one parameter.");
        }
      }
    }

    else
    {
      type = NList::typeError("Operator matchgrid expects "
                              "a t type object or a mt type object "
                              "as first argument.");
    }
  }

  return type;
}

}
