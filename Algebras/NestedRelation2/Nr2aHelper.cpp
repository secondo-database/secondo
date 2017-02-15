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

#include "Include.h"

using namespace std;

namespace nr2a {

/*
The following functions build some less more complex types from the given
list expression containing the resulting type's attributes' types.

*/
/*static*/ ListExpr Nr2aHelper::TupleStreamOf(ListExpr attributesTypes)
{
  return nl->TwoElemList(nl->SymbolAtom(Symbol::STREAM()),
      TupleOf(attributesTypes));
}

/*static*/ ListExpr Nr2aHelper::TupleOf(ListExpr attributesTypes)
{
  return nl->TwoElemList(nl->SymbolAtom("tuple"), attributesTypes);
}

/*static*/ ListExpr Nr2aHelper::RecordOf(ListExpr attributesTypes)
{
  return nl->Cons(nl->SymbolAtom("record"), attributesTypes);
}

/*
To check whether a given type is either a nested relation or an attribute
relation this function can be used.

*/
/*static*/ bool Nr2aHelper::IsNestedRelation(const ListExpr definition)
{
  bool result = false;
  AutoWrite(definition);

  if (!nl->IsEmpty(definition) && nl->HasLength(definition, 2))
  {
    const ListExpr nestedRelationType = nl->First(definition);
    if (listutils::isSymbol(nestedRelationType))
    {
      result = (nl->SymbolValue(nestedRelationType) == ARel::BasicType())
          || (nl->SymbolValue(nestedRelationType) == NRel::BasicType());
    }
    else if (IsNumericRepresentationOf(nestedRelationType, ARel::BasicType())
        || IsNumericRepresentationOf(nestedRelationType, NRel::BasicType()))
    {
      result = true;
    }
  }

  return result;
}

/*
Checks if the first argument is a numeric representation (in nested list
format) of the type given as a string in the second argument.

*/
/*static*/ bool Nr2aHelper::IsNumericRepresentationOf(
    const ListExpr list, const string typeName)
{
  int algId = 0;
  int typeId = 0;
  bool result = false;
  if (nl->HasLength(list, 2) && (nl->AtomType(nl->First(list))==IntType)
      && (nl->AtomType(nl->Second(list))==IntType))
  {
    SecondoSystem::GetCatalog()->GetTypeId(typeName, algId, typeId);
    result = (algId == nl->IntValue(nl->First(list))) &&
        (typeId == nl->IntValue(nl->Second(list)));
  }
  return result;
}

/*
A few operators in the algebra can process both, "ARel"[2] and "NRel"[2]
values. They can reference this method instead of defining their implementation
of the same logic.

*/
/*static*/ int Nr2aHelper::DefaultSelect(const ListExpr type)
{
  // When using this function it is assumed, that only arel2 and nrel2 are
  // accepted by the type mapping function
  AutoWrite(type);
  assert(
      listutils::isSymbol(nl->First(type), ARel::BasicType())
          || listutils::isSymbol(nl->First(type), NRel::BasicType()));
  return (nl->SymbolValue(nl->First(type)) == ARel::BasicType()) ? 0 : 1;
}

/*static*/ string Nr2aHelper::IntToString(const int num)
{
  const int cBufferLen = 10;
  char buffer[cBufferLen];
  memset(buffer, '\0', cBufferLen);
  sprintf(buffer, "%d", num);
  return string(buffer);
}

/*static*/ double Nr2aHelper::MillisecondsElapsedSince
    (clock_t previousClock)
{
  clock_t now = clock();
  assert(previousClock <= now);
  return ((double)(now - previousClock) * 1000) / CLOCKS_PER_SEC;
}

/*
A list builder is constructed with an empty list in its storage...

*/
ListBuilder::ListBuilder()
    : m_list(nl->TheEmptyList()), m_end(nl->TheEmptyList())
{
}

/*
...which can then be filled via appending nested list expression or attribute
definitions, which are then transformed to nested list format internally.

*/
void ListBuilder::Append(const ListExpr newElement)
{
  if (nl->IsEmpty(m_list))
  {
    m_list = nl->OneElemList(newElement);
    m_end = m_list;
  }
  else
  {
    m_end = nl->Append(m_end, newElement);
  }
}

void ListBuilder::AppendAttribute(const string attributeName,
    const ListExpr type)
{
  Append(nl->TwoElemList(nl->SymbolAtom(attributeName), type));
}

void ListBuilder::AppendAttribute(const string attributeName,
    const string typeName)
{
  AppendAttribute(attributeName, nl->SymbolAtom(typeName));
}

/*
If the content is defined properly one can request a type definition of the
desired type by calling one of the provided getters.

*/
ListExpr ListBuilder::GetList() const
{
  return m_list;
}

ListExpr ListBuilder::GetTuple() const
{
  return Nr2aHelper::TupleOf(m_list);
}

ListExpr ListBuilder::GetRecord() const
{
  return Nr2aHelper::RecordOf(m_list);
}

ListExpr ListBuilder::GetARel() const
{
  return nl->TwoElemList(nl->SymbolAtom(ARel::BasicType()),
      Nr2aHelper::TupleOf(m_list));
}

ListExpr ListBuilder::GetNRel() const
{
  return nl->TwoElemList(nl->SymbolAtom(NRel::BasicType()),
      Nr2aHelper::TupleOf(m_list));
}

ListExpr ListBuilder::GetTupleStream() const
{
  return Nr2aHelper::TupleStreamOf(m_list);
}

} // namespace nr2a
