/*
----
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science,
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
----

*/

#include "BoxIntersects.h"

#include "AttrArray.h"
#include <cstdint>
#include <exception>
#include "Ints.h"
#include "LongIntsTC.h"
#include "LogMsg.h"
#include "OperatorUtils.h"
#include "QueryProcessor.h"
#include "RectangleAlgebra.h"
#include "SpatialAttrArray.h"
#include <string>
#include "Symbols.h"

using namespace CRelAlgebra;
using namespace CRelAlgebra::Operators;

using std::exception;
using std::string;
using std::vector;

extern NestedList *nl;
extern QueryProcessor *qp;

//BoxIntersects-----------------------------------------------------------------

BoxIntersects::BoxIntersects() :
  Operator(info, valueMappings, SelectValueMapping, TypeMapping)
{
}

ValueMapping BoxIntersects::valueMappings[] =
{
  AttrArrayValueMapping<1, 1>,
  AttrArrayValueMapping<1, 2>,
  AttrArrayValueMapping<1, 3>,
  AttrArrayValueMapping<1, 4>,
  AttrArrayValueMapping<1, 8>,

  AttributeValueMapping<1, 1>,
  AttributeValueMapping<1, 2>,
  AttributeValueMapping<1, 3>,
  AttributeValueMapping<1, 4>,
  AttributeValueMapping<1, 8>,

  AttrArrayValueMapping<2, 1>,
  AttrArrayValueMapping<2, 2>,
  AttrArrayValueMapping<2, 3>,
  AttrArrayValueMapping<2, 4>,
  AttrArrayValueMapping<2, 8>,
  
  AttributeValueMapping<2, 1>,
  AttributeValueMapping<2, 2>,
  AttributeValueMapping<2, 3>,
  AttributeValueMapping<2, 4>,
  AttributeValueMapping<2, 8>,

  AttrArrayValueMapping<3, 1>,
  AttrArrayValueMapping<3, 2>,
  AttrArrayValueMapping<3, 3>,
  AttrArrayValueMapping<3, 4>,
  AttrArrayValueMapping<3, 8>,
  
  AttributeValueMapping<3, 1>,
  AttributeValueMapping<3, 2>,
  AttributeValueMapping<3, 3>,
  AttributeValueMapping<3, 4>,
  AttributeValueMapping<3, 8>,

  AttrArrayValueMapping<4, 1>,
  AttrArrayValueMapping<4, 2>,
  AttrArrayValueMapping<4, 3>,
  AttrArrayValueMapping<4, 4>,
  AttrArrayValueMapping<4, 8>,

  AttributeValueMapping<4, 1>,
  AttributeValueMapping<4, 2>,
  AttributeValueMapping<4, 3>,
  AttributeValueMapping<4, 4>,
  AttributeValueMapping<4, 8>,

  AttrArrayValueMapping<8, 1>,
  AttrArrayValueMapping<8, 2>,
  AttrArrayValueMapping<8, 3>,
  AttrArrayValueMapping<8, 4>,
  AttrArrayValueMapping<8, 8>,
  
  AttributeValueMapping<8, 1>,
  AttributeValueMapping<8, 2>,
  AttributeValueMapping<8, 3>,
  AttributeValueMapping<8, 4>,
  AttributeValueMapping<8, 8>,
};

const OperatorInfo BoxIntersects::info =
  OperatorInfo("boxintersects", "SPATIALATTRARRAY x "
               "(SPATIALATTRARRAY | SPATIAL) -> longints",
               "_ boxintersects _",
               "Determines ascending sequence of indices of those entries in "
               "the first array whichs bounding boxe intersects:\n"
               "a) the bounding box of the entry in the second array at the "
               "same index.\n"
               "b) the second argument's bounding box.\n\n"
               "Bounding boxes are projected on the lower dimension.",
               "query cstrassen feed filter[.GeoData boxintersects [const rect "
               "value (0 0 0 0)]] count");

ListExpr BoxIntersects::TypeMapping(ListExpr args)
{
  if(!nl->HasLength(args,2))
  {
    return GetTypeError("Two arguments expected.");
  }
  
  TypeConstructor &typeConstructorA = *GetTypeConstructor(nl->First(args));

  if (IsSpatialAttrArray(typeConstructorA))
  {
    TypeConstructor &typeConstructorB = *GetTypeConstructor(nl->First(args));
    
    if (!IsSpatialAttrArray(typeConstructorB) &&
        !IsSpatialAttr(typeConstructorB))
    {
      return GetTypeError(1, "b",
                          "Not of kind SPATIAL1D, SPATIAL2D, SPATIAL3D, "
                          "SPATIAL4D, SPATIAL8D, SPATIALATTRARRAY1D, "
                          "SPATIALATTRARRAY2D, "
                          "SPATIALATTRARRAY3D, SPATIALATTRARRAY4D, or "
                          "SPATIALATTRARRAY8D.");
    }
  }
  else
  {
    return GetTypeError(0, "a",
                        "Not of kind SPATIALATTRARRAY1D, "
                        "SPATIALATTRARRAY2D, "
                        "SPATIALATTRARRAY3D, SPATIALATTRARRAY4D, or "
                        "SPATIALATTRARRAY8D.");
  }
  
  //Result type is 'longints'
  return LongIntsTI(false).GetTypeExpr();
}

int BoxIntersects::SelectValueMapping(ListExpr args)
{
  const vector<string> attrArrayKinds = { Kind::SPATIALATTRARRAY1D(),
                                          Kind::SPATIALATTRARRAY2D(),
                                          Kind::SPATIALATTRARRAY3D(),
                                          Kind::SPATIALATTRARRAY4D(),
                                          Kind::SPATIALATTRARRAY8D() };

  const vector<string> attrKinds = { Kind::SPATIAL1D(),
                                     Kind::SPATIAL2D(),
                                     Kind::SPATIAL3D(),
                                     Kind::SPATIAL4D(),
                                     Kind::SPATIAL8D() };

  int result = 0;

  TypeConstructor &typeConstructorA = *GetTypeConstructor(nl->First(args));
  
  for (const string &kind : attrArrayKinds)
  {
    if (typeConstructorA.MemberOf(kind))
    {
      break;
    }

    result += 10;
  }

  TypeConstructor &typeConstructorB = *GetTypeConstructor(nl->Second(args));
  
  for (const string &kind : attrArrayKinds)
  {
    if (typeConstructorB.MemberOf(kind))
    {
      return result;
    }

    ++result;
  }

  for (const string &kind : attrKinds)
  {
    if (typeConstructorB.MemberOf(kind))
    {
      return result;
    }

    ++result;
  }

  return -1;
}

template<int dimA, int dimB>
int BoxIntersects::AttributeValueMapping(ArgVector args, Word &result, 
                                         int message, Word &local, Supplier s)
{
  static const int minDim = dimA < dimB ? dimA : dimB;

  try
  {
    SpatialAttrArray<dimA> &values = *(SpatialAttrArray<dimA>*)args[0].addr;
    LongInts &indices = qp->ResultStorage<LongInts>(result, s);

    indices.Clear();

    StandardSpatialAttribute<dimB> &value =
      *(StandardSpatialAttribute<dimB>*)args[1].addr;

    if (value.IsDefined())
    {
      Rectangle<dimB> boxB = value.BoundingBox();

      if (dimB != minDim)
      {
        const Rectangle<minDim> boxBMin = boxB. template project<minDim>();

        for (const AttrArrayEntry &entry : values.GetFilter())
        {
          const int64_t row = entry.GetRow();

          if (entry.IsDefined())
          {
            Rectangle<dimA> boxA = values.GetBoundingBox(row);

            if (((const Rectangle<minDim>&)boxA).Intersects(boxBMin))
            {
              indices.Append(row);
            }
          }
        }
      }
      else
      {
        for (const AttrArrayEntry &entry : values.GetFilter())
        {
          const int64_t row = entry.GetRow();

          if (entry.IsDefined())
          {
            Rectangle<dimA> boxA = values.GetBoundingBox(row);

            if (dimA != minDim)
            {
              if (boxA. template project<minDim>().Intersects(
                (const Rectangle<minDim>&)boxB))
              {
                indices.Append(row);
              }
            }
            else
            {
              if (((const Rectangle<minDim>&)boxA).Intersects(
                (const Rectangle<minDim>&)boxB))
              {
                indices.Append(row);
              }
            }
          }
        }
      }
    }

    return 0;
  }
  catch (const exception &e)
  {
    ErrorReporter::ReportError(e.what());
  }

  return FAILURE;
}

template<int dimA, int dimB>
int BoxIntersects::AttrArrayValueMapping(ArgVector args, Word &result,
                                         int message, Word &local, Supplier s)
{
  static const int minDim = dimA < dimB ? dimA : dimB;

  try
  {
    LongInts &indices = qp->ResultStorage<LongInts>(result, s);

    indices.Clear();

    SpatialAttrArray<dimA> &valuesA = *(SpatialAttrArray<dimA>*)args[0].addr;

    SpatialAttrArray<dimB> &valuesB = *(SpatialAttrArray<dimB>*)args[1].addr;

    FilteredAttrArrayIterator iteratorA = valuesA.GetFilteredIterator(),
      iteratorB = valuesB.GetFilteredIterator();

    if (iteratorA.IsValid() && iteratorB.IsValid())
    {
      if (dimB != minDim)
      {
        do
        {
          const AttrArrayEntry &a = iteratorA.Get(),
            &b = iteratorB.Get();
  
          if (!a.IsDefined() || !b.IsDefined())
          {
            continue;
          }
  
          const int64_t rowA = a.GetRow();
  
          Rectangle<dimA> boxA = valuesA.GetBoundingBox(rowA);
  
          Rectangle<minDim> boxB =
            valuesB.GetBoundingBox(b.GetRow()). template project<minDim>();
  
          if (((const Rectangle<minDim>&)boxA).Intersects(boxB))
          {
            indices.Append(rowA);
          }
        }
        while (iteratorA.MoveToNext() && iteratorB.MoveToNext());
      }
      else
      {
        do
        {
          const AttrArrayEntry &a = iteratorA.Get(),
            &b = iteratorB.Get();
  
          if (!a.IsDefined() || !b.IsDefined())
          {
            continue;
          }
  
          const int64_t rowA = a.GetRow();
  
          Rectangle<dimA> boxA = valuesA.GetBoundingBox(rowA);
  
          Rectangle<dimB> boxB = valuesB.GetBoundingBox(b.GetRow());
  
          if (dimA != minDim)
          {
            if (boxA. template project<minDim>().Intersects(
              (const Rectangle<minDim>&)boxB))
            {
              indices.Append(rowA);
            }
          }
          else
          {
            if (((const Rectangle<minDim>&)boxA).Intersects(
              (const Rectangle<minDim>&)boxB))
            {
              indices.Append(rowA);
            }
          }
        }
        while (iteratorA.MoveToNext() && iteratorB.MoveToNext());
      }
    }

    return 0;
  }
  catch (const exception &e)
  {
    ErrorReporter::ReportError(e.what());
  }

  return FAILURE;
}