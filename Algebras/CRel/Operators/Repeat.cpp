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

#include <exception>
#include "ListUtils.h"
#include "LogMsg.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"
#include "StreamValueMapping.h"

using namespace CRelAlgebra::Operators;

using listutils::isDATA;
using listutils::isTupleDescription;
using std::exception;

extern QueryProcessor *qp;

Repeat::Repeat() :
  Operator(info, valueMappings, SelectValueMapping, TypeMapping)
{
  SetUsesArgsInTypeMapping();
}

const OperatorInfo Repeat::info = OperatorInfo(
  "repeat",
  "DATA -> stream(DATA)",
  "repeat(_,_)",
  "Creates a stream by repeating the value the specified numer of times.",
  "query repeat('Test', 2) transformstream consume");

ValueMapping Repeat::valueMappings[] =
{
  StreamValueMapping<AttributeState>,
  StreamValueMapping<TupleState>,
  StreamValueMapping<StreamState<Attribute>>,
  StreamValueMapping<StreamState<Tuple>>,
  NULL
};

ListExpr Repeat::TypeMapping(ListExpr args)
{
  if (!nl->HasLength(args, 2))
  {
    return listutils::typeError("Expected two arguments.");
  }

  const ListExpr firstArg = nl->First(nl->First(args));

  const bool isStream = nl->HasLength(firstArg, 2) &&
                        nl->IsEqual(nl->First(firstArg), Symbol::STREAM());

  const ListExpr attributeType = isStream ? nl->Second(firstArg) : firstArg;
  if (!isDATA(attributeType) && !isTupleDescription(attributeType))
  {
    return listutils::typeError("First argument is not a (stream of) tuple or "
                                "of kind DATA.");
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

  return nl->TwoElemList(nl->SymbolAtom(Symbols::STREAM()), attributeType);
}

int Repeat::SelectValueMapping(ListExpr args)
{
  const ListExpr firstArg = nl->First(args);

  if (nl->HasLength(firstArg, 2) &&
      nl->IsEqual(nl->First(firstArg), Symbol::STREAM()))
  {
    return isDATA(nl->Second(firstArg)) ? 2 : 3;
  }
  else
  {
    return isDATA(firstArg) ? 0 : 1;
  }
}


Repeat::AttributeState::AttributeState(Word *args, Supplier s) :
  m_count(((CcInt*)args[1].addr)->GetValue()),
  m_index(0),
  m_attribute(*(Attribute*)args[0].addr)
{
  qp->DeleteResultStorage(s);
}

Attribute *Repeat::AttributeState::Request()
{
  if (m_index++ < m_count)
  {
    return m_attribute.Clone();
  }

  return NULL;
}


Repeat::TupleState::TupleState(Word *args, Supplier s) :
  m_count(((CcInt*)args[1].addr)->GetValue()),
  m_index(0),
  m_tuple(*(Tuple*)args[0].addr)
{
  qp->DeleteResultStorage(s);
}

Tuple *Repeat::TupleState::Request()
{
  if (m_index++ < m_count)
  {
    m_tuple.IncReference();

    return &m_tuple;
  }

  return NULL;
}

template<class T>
Repeat::StreamState<T>::StreamState(Word *args, Supplier s) :
  m_count(((CcInt*)args[1].addr)->GetValue()),
  m_index(0),
  m_stream(args[0].addr)
{
  qp->DeleteResultStorage(s);

  m_stream.open();
}

template<class T>
Repeat::StreamState<T>::~StreamState()
{
  m_stream.close();
}

template<class T>
T *Repeat::StreamState<T>::Request()
{
  T *value = m_stream.request();

  if (value == NULL)
  {
    if (++m_index < m_count)
    {
      m_stream.close();
      m_stream.open();

      if ((value = m_stream.request()) != NULL)
      {
        return value;
      }
    }

    return NULL;
  }

  return value;
}