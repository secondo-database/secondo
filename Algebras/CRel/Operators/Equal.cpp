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

#include "Equal.h"

#include "AttrArray.h"
#include <cstddef>
#include <exception>
#include "GAttrArray.h"
#include "GAttrArrayTI.h"
#include "Ints.h"
#include "IntsTI.h"
#include "LongInts.h"
#include "LongIntsTI.h"
#include "ListUtils.h"
#include "LogMsg.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include "TypeConstructor.h"
#include "TypeUtils.h"

#include <iostream>

using namespace CRelAlgebra;
using namespace CRelAlgebra::Operators;

using std::exception;

extern NestedList *nl;
extern QueryProcessor *qp;

//ApplyPredicate----------------------------------------------------------------

Equal::Equal() :
  Operator(info, valueMappings, SelectValueMapping, TypeMapping)
{
}

ValueMapping Equal::valueMappings[] =
{
  DefaultValueMapping,
  IntsValueMapping,
  Ints2ValueMapping,
  GAttrArrayValueMapping<CcInt>,
  nullptr
};

const OperatorInfo Equal::info = OperatorInfo(
  "=", "ATTRARRAY x ATTRIBUTETYPE -> longints",
  "a = v",
  "Determines the indices of those entries in the array a whose attribute "
  "representations evaluate to equal to the provided value v.\n"
  "The returned indices are in ascending order.",
  "query people feed filter[.BirthYear = 1991] count");

ListExpr Equal::TypeMapping(ListExpr args)
{
  //Expect two arguments
  if(!nl->HasLength(args,2))
  {
    return listutils::typeError("Two arguments expected.");
  }

  const ListExpr firstArg = nl->First(args);

  TypeConstructor *typeConstructor = GetTypeConstructor(firstArg);

  if (typeConstructor == nullptr ||
      !typeConstructor->MemberOf(Kind::ATTRARRAY()))
  {
    return listutils::typeError("First argument isn't of kind ATTRARRAY.");
  }

  const ListExpr attributeType =
    ((AttrArrayTypeConstructor*)typeConstructor)->GetAttributeType(firstArg,
                                                                   false);

  if (!nl->Equal(attributeType, nl->Second(args)))
  {
    return listutils::typeError("Second argument isn't of first argument's "
                                "attribute type.");
  }

  //Result type is 'longints'
  return LongIntsTI(false).GetTypeExpr();
}

int Equal::SelectValueMapping(ListExpr args)
{
  return 0;

  const ListExpr firstArg = nl->First(args);

  if (GAttrArrayTI::Check(firstArg))
  {
    const ListExpr attributeType =
      GAttrArrayTI(firstArg, false).GetAttributeType();

    if (CcInt::checkType(attributeType))
    {
      return 3;
    }
  }

  if (Ints2TI::Check(firstArg))
  {
    return 2;
  }

  if (IntsTI::Check(firstArg))
  {
    return 1;
  }

  return 0;
}

int Equal::DefaultValueMapping(ArgVector args, Word &result, int message,
                               Word &local, Supplier s)
{
  try
  {
    AttrArray &values = *(AttrArray*)args[0].addr;
    LongInts &indices = qp->ResultStorage<LongInts>(result, s);

    indices.Clear();

    Attribute &value = *(Attribute*)args[1].addr;

    size_t i = 0;

    if (value.IsDefined())
    {
      for (const AttrArrayEntry &entry : values)
      {
        if (entry.Equals(value))
        {
          indices.Append(i);
        }

        ++i;
      }
    }
    else
    {
      for (const AttrArrayEntry &entry : values)
      {
        if (!entry.IsDefined())
        {
          indices.Append(i);
        }

        ++i;
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

template<class T>
int Equal::GAttrArrayValueMapping(ArgVector args, Word &result, int message,
                                  Word &local, Supplier s)
{
  try
  {
    GAttrArray &values = *(GAttrArray*)args[0].addr;
    LongInts &indices = qp->ResultStorage<LongInts>(result, s);

    indices.Clear();

    T &value = *(T*)args[1].addr;

    int64_t i = 0;

    if (value.IsDefined())
    {
      for (Attribute &entry : values)
      {
        if (((T&)entry).Equal(&value))
        {
          indices.Append(i);
        }

        ++i;
      }
    }
    else
    {
      for (Attribute &entry : values)
      {
        if (!((T&)entry).IsDefined())
        {
          indices.Append(i);
        }

        ++i;
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

int Equal::IntsValueMapping(ArgVector args, Word &result, int message,
                            Word &local, Supplier s)
{
  try
  {
    Ints &values = *(Ints*)args[0].addr;
    LongInts &indices = qp->ResultStorage<LongInts>(result, s);

    indices.Clear();

    CcInt &value = *(CcInt*)args[1].addr;

    int64_t i = 0;

    if (value.IsDefined())
    {
      const int32_t intValue = value.GetValue();

      for (const IntEntry &entry : values)
      {
        if (entry.IsDefined() && entry == intValue)
        {
          indices.Append(i);
        }

        ++i;
      }
    }
    else
    {
      for (const IntEntry &entry : values)
      {
        if (!entry.IsDefined())
        {
          indices.Append(i);
        }

        ++i;
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

int Equal::Ints2ValueMapping(ArgVector args, Word &result, int message,
                             Word &local, Supplier s)
{
  try
  {
    Ints2 &values = *(Ints2*)args[0].addr;
    LongInts &indices = qp->ResultStorage<LongInts>(result, s);

    indices.Clear();

    CcInt &value = *(CcInt*)args[1].addr;

    int64_t i = 0;

    if (value.IsDefined())
    {
      for (const IntEntry2 &entry : values)
      {
        if (entry.Equals(value))
        {
          indices.Append(i);
        }

        ++i;
      }
    }
    else
    {
      for (const IntEntry2 &entry : values)
      {
        if (!entry.IsDefined())
        {
          indices.Append(i);
        }

        ++i;
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