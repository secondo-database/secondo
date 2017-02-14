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

#include "IndicesTC.h"

#include <exception>
#include "ListUtils.h"
#include "LogMsg.h"
#include "Utility.h"

using namespace CRelAlgebra;
using namespace listutils;

using std::exception;
using std::string;
using std::vector;

extern NestedList *nl;

const string IndicesTC::name = "intset";

ListExpr IndicesTC::TypeProperty()
{
  return nl->TwoElemList(nl->FourElemList(nl->StringAtom("Signature"),
                                          nl->StringAtom("Example Type List"),
                                          nl->StringAtom("List Rep"),
                                          nl->StringAtom("Example List")),
                         nl->FourElemList(nl->StringAtom(""),
                                          nl->StringAtom(""),
                                          nl->TextAtom(""),
                                          nl->TextAtom("")));
}

bool IndicesTC::CheckType(ListExpr typeExpr, ListExpr &errorInfo)
{
  std::string error;
  if (!IndicesTI::Check(typeExpr, error))
  {
    errorInfo = simpleMessage(error);
    return false;
  }

  return true;
}

Word IndicesTC::In(ListExpr typeExpr, ListExpr value, int, ListExpr &errorInfo,
                  bool &correct)
{
  correct = true;

  try
  {
    vector<size_t> *result = new vector<size_t>();

    if (nl->IsAtom(value))
    {
      ErrorReporter::ReportError("Expected list of ints.");
      return Word();
    }

    ListExpr ints = value;
    while (!nl->IsEmpty(ints))
    {
      const ListExpr intAtom = nl->First(ints);
      ints = nl->Rest(ints);

      if (!nl->IsNodeType(IntType, intAtom))
      {
        ErrorReporter::ReportError("Expected list of ints.");
        return Word();
      }

      result->push_back(nl->IntValue(intAtom));
    }

    return Word(result);
  }
  catch (const exception &e)
  {
    errorInfo = simpleMessage(e.what());
    correct = false;
    return Word();
  }
}

ListExpr IndicesTC::Out(ListExpr typeExpr, Word value)
{
  const ListExpr values = nl->OneElemList(nl->Empty());
  ListExpr valuesEnd = values;

  for (size_t index : *(vector<size_t>*)value.addr)
  {
    valuesEnd = nl->Append(valuesEnd, nl->IntAtom(index));
  }

  return nl->Rest(values);
}


Word IndicesTC::Create(const ListExpr typeExpr)
{
  return Word(new vector<size_t>());
}

void IndicesTC::Delete(const ListExpr, Word &value)
{
  delete (vector<size_t>*)value.addr;

  value.addr = NULL;
}

bool IndicesTC::Open(SmiRecord &valueRecord, size_t &offset,
                     const ListExpr typeExpr, Word &value)
{
  try
  {
    SmiReader source = SmiReader(valueRecord, offset);

    const size_t count = source.ReadOrThrow<size_t>();

    vector<size_t> *indices = new vector<size_t>(count);

    for (size_t i = 0; i < count; ++i)
    {
      indices->push_back(source.ReadOrThrow<long>());
    }

    offset = source.GetPosition();

    value = Word(indices);

    return true;
  }
  catch (const exception &e)
  {
    value = Word();

    return false;
  }
}

bool IndicesTC::Save(SmiRecord &valueRecord, size_t &offset, const ListExpr,
                     Word &value)
{
  try
  {
    SmiWriter target = SmiWriter(valueRecord, offset);

    vector<size_t> &instance = *(vector<size_t>*)value.addr;

    const size_t count = instance.size();

    target.WriteOrThrow(count);
    target.WriteOrThrow((char*)&instance.front(), count * sizeof(size_t));

    offset = target.GetPosition();

    return true;
  }
  catch (const exception &e)
  {
    return false;
  }
}

void IndicesTC::Close(const ListExpr, Word &w)
{
  delete (vector<size_t>*)w.addr;

  w.addr = NULL;
}

void *IndicesTC::Cast(void *addr)
{
  return new (addr) vector<size_t>;
}

int IndicesTC::SizeOf()
{
  return sizeof(vector<size_t>);
}

Word IndicesTC::Clone(const ListExpr, const Word &w)
{
  return Word(new vector<size_t>(*(vector<size_t>*)w.addr));
}

IndicesTC::IndicesTC() :
  TypeConstructor(name, TypeProperty, Out, In, NULL, NULL, Create, Delete,
                  Open, Save, Close, Clone, Cast, SizeOf, CheckType)
{
}