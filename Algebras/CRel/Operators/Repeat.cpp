/*
----
This file is part of SECONDO.

Copyright (C) 2004-2009, University in Hagen, Faculty of Mathematics
and Computer Science, Database Systems for New Applications.

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

#include "Repeat.h"

#include "ListUtils.h"
#include "LogMsg.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include "StreamValueMapping.h"
#include "TypeUtils.h"

using namespace CRelAlgebra::Operators;

using listutils::isDATA;
using listutils::isStream;
using listutils::isTupleDescription;

extern QueryProcessor *qp;

Repeat::Repeat() :
  Operator(info, StreamValueMapping<State>, TypeMapping)
{
  SetUsesArgsInTypeMapping();
}

const OperatorInfo Repeat::info = OperatorInfo(
  "repeat", "stream(T) x int -> stream(T)",
  "repeat( _, _ )",
  "Creates a stream by repeating a stream the specified number of times.",
  "query repeat('Test' feed, 2) count");

ListExpr Repeat::TypeMapping(ListExpr args)
{
  if (!nl->HasLength(args, 2))
  {
    return listutils::typeError("Expected two arguments.");
  }

  const ListExpr firstArg = nl->First(nl->First(args));

  if (!isStream(firstArg))
  {
    return listutils::typeError("First argument is not a stream.");
  }

  const ListExpr count = nl->Second(args);
  if (!CcInt::checkType(nl->First(count)))
  {
    return listutils::typeError("Second argument is not an int.");
  }

  if (nl->IntValue(nl->Second(count)) < 0)
  {
    return listutils::typeError("Second argument is < 0.");
  }

  return firstArg;
}

Repeat::State::State(Word *args, Supplier s) :
  m_count(((CcInt*)args[1].addr)->GetValue()),
  m_index(0),
  m_stream(args[0].addr)
{
  qp->DeleteResultStorage(s);

  m_stream.open();
}

Repeat::State::~State()
{
  m_stream.close();
}

void *Repeat::State::Request()
{
  void *value = m_stream.request();

  if (value == nullptr)
  {
    if (++m_index < m_count)
    {
      m_stream.close();
      m_stream.open();

      if ((value = m_stream.request()) != nullptr)
      {
        return value;
      }
    }

    return nullptr;
  }

  return value;
}