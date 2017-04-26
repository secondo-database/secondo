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

#include "Compare.h"

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

template<CompareMode mode>
Compare<mode>::Compare() :
  Operator(info, valueMappings, SelectValueMapping, TypeMapping)
{
}

template<CompareMode mode>
ValueMapping Compare<mode>::valueMappings[] =
{
  AttributeValueMapping,
  AttrArrayValueMapping,
  nullptr
};

OperatorInfo GetOperatorInfo(CompareMode mode)
{
  string s0,
    s1;

  switch (mode)
  {
    case CompareMode::Less:
      s0 = "<";
      s1 = "less than";
      break;
    case CompareMode::LessOrEqual:
      s0 = "<=";
      s1 = "less or equal to";
      break;
    case CompareMode::Equal:
      s0 = "==";
      s1 = "equal to";
      break;
    case CompareMode::NotEqual:
      s0 = "#";
      s1 = "not equal to";
      break;
    case CompareMode::GreaterOrEqual:
      s0 = ">=";
      s1 = "greater or equal to";
      break;
    case CompareMode::Greater:
      s0 = ">";
      s1 = "greater than";
      break;
    default:
      return OperatorInfo();
  }

  return OperatorInfo(
    s0,
    "ATTRARRAY x ATTRARRAY -> longints, \n"
    "ATTRARRAY x DATA -> longints",
    "_ " + s0 + " _",
    "Determines ascending sequence of indices of those entries in the first "
    "array which are considered " + s1 + ":\n"
    "a) the entry in the second array at the same index. Both arrays must be "
    "of equal type.\n"
    "b) the second argument's value. The value must be of the arrays "
    "attribute-type\n",
    "query people feed filter[.BirthYear " + s0 + " 1991] count");
}

template<CompareMode mode>
const OperatorInfo Compare<mode>::info = GetOperatorInfo(mode);

template<CompareMode mode>
ListExpr Compare<mode>::TypeMapping(ListExpr args)
{
  //Expect two arguments
  if(!nl->HasLength(args,2))
  {
    return listutils::typeError("Two arguments expected.");
  }

  const ListExpr firstArg = nl->First(args);

  TypeConstructor *typeConstructorA = GetTypeConstructor(firstArg);

  if (typeConstructorA == nullptr ||
      !typeConstructorA->MemberOf(Kind::ATTRARRAY()))
  {
    return listutils::typeError("First argument isn't of kind ATTRARRAY.");
  }

  const ListExpr secondArg = nl->Second(args);

  TypeConstructor *typeConstructorB = GetTypeConstructor(secondArg);

  if (typeConstructorB == nullptr)
  {
    return listutils::typeError("First argument's type could not be "
                                "determined.");
  }

  if (!typeConstructorB->MemberOf(Kind::ATTRARRAY()) ||
      !nl->Equal(firstArg, secondArg))
  {
    const ListExpr attributeType =
      ((AttrArrayTypeConstructor*)typeConstructorA)->GetAttributeType(firstArg,
                                                                      false);

    if (!nl->Equal(attributeType, secondArg))
    {
      return listutils::typeError("Second argument is neither of the first "
                                  "argument's type nor of it's attribute "
                                  "type.");
    }
  }

  //Result type is 'longints'
  return LongIntsTI(false).GetTypeExpr();
}

template<CompareMode mode>
int Compare<mode>::SelectValueMapping(ListExpr args)
{
  return CheckKind(Kind::ATTRARRAY(), nl->Second(args)) ? 1 : 0;
}

template<CompareMode mode>
int Compare<mode>::AttributeValueMapping(ArgVector args, Word &result,
                                         int message, Word &local, Supplier s)
{
  try
  {
    AttrArray &values = *(AttrArray*)args[0].addr;
    LongInts &indices = qp->ResultStorage<LongInts>(result, s);

    indices.Clear();

    Attribute &value = *(Attribute*)args[1].addr;

    if (value.IsDefined())
    {
      for (const AttrArrayEntry &entry : values)
      {
        if ((mode == CompareMode::Less && entry < value) ||
            (mode == CompareMode::LessOrEqual && entry <= value) ||
            (mode == CompareMode::Equal && entry == value) ||
            (mode == CompareMode::NotEqual && entry != value) ||
            (mode == CompareMode::GreaterOrEqual && entry >= value) ||
            (mode == CompareMode::Greater && entry > value))
        {
          indices.Append(entry.GetRow());
        }
      }
    }
    else if (mode != CompareMode::Less)
    {
      for (const AttrArrayEntry &entry : values)
      {
        if ((mode == CompareMode::LessOrEqual && !entry.IsDefined()) ||
            (mode == CompareMode::Equal && !entry.IsDefined()) ||
            (mode == CompareMode::NotEqual && entry.IsDefined()) ||
            (mode == CompareMode::GreaterOrEqual) ||
            (mode == CompareMode::Greater && entry.IsDefined()))
        {
          indices.Append(entry.GetRow());
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

template<CompareMode mode>
int Compare<mode>::AttrArrayValueMapping(ArgVector args, Word &result,
                                         int message, Word &local, Supplier s)
{
  try
  {
    LongInts &indices = qp->ResultStorage<LongInts>(result, s);

    indices.Clear();

    AttrArrayIterator iteratorA = (*(AttrArray*)args[0].addr).GetIterator(),
      iteratorB = (*(AttrArray*)args[1].addr).GetIterator();

    if (iteratorA.IsValid() && iteratorB.IsValid())
    {
      do
      {
        const AttrArrayEntry &a = iteratorA.Get(),
          &b = iteratorB.Get();

        if ((mode == CompareMode::Less && a < b) ||
            (mode == CompareMode::LessOrEqual && a <= b) ||
            (mode == CompareMode::Equal && a == b) ||
            (mode == CompareMode::NotEqual && a != b) ||
            (mode == CompareMode::GreaterOrEqual && a >= b) ||
            (mode == CompareMode::Greater && a > b))
        {
          indices.Append(a.GetRow());
        }
      }
      while (iteratorA.MoveToNext() && iteratorB.MoveToNext());
    }

    return 0;
  }
  catch (const exception &e)
  {
    ErrorReporter::ReportError(e.what());
  }

  return FAILURE;
}

template class Compare<CompareMode::NotEqual>;
template class Compare<CompareMode::Less>;
template class Compare<CompareMode::LessOrEqual>;
template class Compare<CompareMode::Equal>;
template class Compare<CompareMode::GreaterOrEqual>;
template class Compare<CompareMode::Greater>;