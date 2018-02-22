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

#include "Distance.h"

#include "AttrArray.h"
#include <cstdint>
#include <exception>
#include "ListUtils.h"
#include "LogMsg.h"
#include "OperatorUtils.h"
#include "QueryProcessor.h"
#include "Algebras/Rectangle/RectangleAlgebra.h"
#include "SpatialAttrArray.h"
#include "StandardTypes.h"
#include <string>
#include "TypeConstructor.h"
#include "TypeUtils.h"

using namespace CRelAlgebra;
using namespace CRelAlgebra::Operators;

using std::exception;
using std::string;

extern NestedList *nl;
extern QueryProcessor *qp;

//ApplyPredicate----------------------------------------------------------------

Distance::Distance() :
  Operator(info, valueMappings, SelectValueMapping, TypeMapping)
{
}

ValueMapping Distance::valueMappings[] =
{
  RectangleValueMapping<1>,
  RectangleValueMapping<2>,
  RectangleValueMapping<3>,
  RectangleValueMapping<4>,
  RectangleValueMapping<8>,
  nullptr
};

const OperatorInfo Distance::info = OperatorInfo(
    "distance",
    "SPATIALATTRARRAY<d> x rect<d> -> ATTRARRAY<real>",
    "distance ( _, _ )",
    "Determines the distance to a rectangle for each entry of the array.",
    "query cstrassen feed filter[distance(.GeoData, [const rect value "
    "(0.0 0.0 0.0 0.0)]) < 500] count");

ListExpr Distance::TypeMapping(ListExpr args)
{
  //Expect two arguments
  if(!nl->HasLength(args,2))
  {
    return GetTypeError("Two arguments expected.");
  }

  const ListExpr firstArg = nl->First(args);

  AttrArrayTypeConstructor *typeConstructorA =
    AttrArray::GetTypeConstructor(firstArg);

  int dimension;

  if (typeConstructorA == nullptr ||
      !IsSpatialAttrArray(*typeConstructorA, dimension))
  {
    return GetTypeError(0, "Isn't of kind SPATIALATTRARRAY1D, "
                           "SPATIALATTRARRAY2D, SPATIALATTRARRAY3D, "
                           "SPATIALATTRARRAY4D or SPATIALATTRARRAY8D.");
  }

  switch (dimension)
  {
  case 1:
    if (!Rectangle<1>::checkType(nl->Second(args)))
    {
      return GetTypeError(1, "Isn't of type " + Rectangle<1>::BasicType() +".");
    }
    break;
  case 2:
    if (!Rectangle<2>::checkType(nl->Second(args)))
    {
      return GetTypeError(1, "Isn't of type " + Rectangle<2>::BasicType() +".");
    }
    break;
  case 3:
    if (!Rectangle<3>::checkType(nl->Second(args)))
    {
      return GetTypeError(1, "Isn't of type " + Rectangle<3>::BasicType() +".");
    }
    break;
  case 4:
    if (!Rectangle<4>::checkType(nl->Second(args)))
    {
      return GetTypeError(1, "Isn't of type " + Rectangle<4>::BasicType() +".");
    }
    break;
  case 8:
    if (!Rectangle<8>::checkType(nl->Second(args)))
    {
      return GetTypeError(1, "Isn't of type " + Rectangle<8>::BasicType() +".");
    }
    break;
  }

  return AttrArrayTypeConstructor::GetDefaultAttrArrayType(
    nl->SymbolAtom(CcReal::BasicType()), false);
}

int Distance::SelectValueMapping(ListExpr args)
{
  const string
    kinds[5] = { Kind::SPATIALATTRARRAY1D(), Kind::SPATIALATTRARRAY2D(),
                 Kind::SPATIALATTRARRAY3D(), Kind::SPATIALATTRARRAY4D(),
                 Kind::SPATIALATTRARRAY8D() };

  TypeConstructor &constructor = *GetTypeConstructor(nl->First(args));

  for (int i = 0; i < 5; ++i)
  {
    if (constructor.MemberOf(kinds[i]))
    {
      return i;
    }
  }

  return -1;
}

template <int dim>
int Distance::RectangleValueMapping(ArgVector args, Word &result,
                                    int message, Word &local, Supplier s)
{
  try
  {
    SpatialAttrArray<dim> &values = *(SpatialAttrArray<dim>*)args[0].addr;
    AttrArray &distances = qp->ResultStorage<AttrArray>(result, s);

    distances.Clear();

    Rectangle<dim> &value = *(Rectangle<dim>*)args[1].addr;

    CcReal distance = CcReal(false);

    if (value.IsDefined())
    {
      for (const SpatialAttrArrayEntry<dim> &entry : values.GetFilter())
      {
        distance.Set(entry.GetDistance(value));

        distances.Append(distance);
      }
    }
    else
    {
      const uint64_t count = values.GetCount();

      for (uint64_t i = 0; i < count; ++i)
      {
        distances.Append(distance);
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
