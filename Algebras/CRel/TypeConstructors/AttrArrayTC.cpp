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

#include "AttrArrayTC.h"

#include <algorithm>
#include "AlgebraManager.h"
#include <exception>
#include "ListUtils.h"
#include <stdexcept>
#include "SecondoSMI.h"
#include "SecondoSystem.h"
#include "Symbols.h"
#include "Utility.h"

using namespace CRelAlgebra;
using namespace listutils;

using std::exception;
using std::max;
using std::runtime_error;
using std::string;

extern NestedList *nl;
extern AlgebraManager *am;

const string AttrArrayTC::name = "attrblock";

ListExpr AttrArrayTC::TypeProperty()
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

bool AttrArrayTC::CheckType(ListExpr typeExpr, ListExpr &errorInfo)
{
  std::string error;
  if (!AttrArrayTI::Check(typeExpr, error))
  {
    errorInfo = simpleMessage(error);
    return false;
  }

  return true;
}

Word AttrArrayTC::In(ListExpr typeExpr, ListExpr value, int,
                     ListExpr &errorInfo, bool &correct)
{
  try
  {
    AttrArrayTI typeInfo = AttrArrayTI(typeExpr);

    GenericAttrArray::PInfo pInfo = typeInfo.GetPInfo();

    SmiFileId flobFileId =
      SecondoSystem::GetCatalog()->GetFlobFile()->GetFileId();

    ListExpr attributeValues = value;

    GenericAttrArray *result = new GenericAttrArray(pInfo, flobFileId);

    if (!nl->IsEmpty(value))
    {
      AttributeInFunction inFunction(am->InObj(pInfo->attributeAlgebraId,
                                              pInfo->attributeTypeId),
                                    typeInfo.GetAttributeType());

      size_t attributeIndex = 0;
      while (!nl->IsEmpty(attributeValues))
      {
        ListExpr error;
        bool correct;

        Attribute *attribute = inFunction(Take(attributeValues),
                                          attributeIndex++, error, correct);

        if (!correct)
        {
          throw runtime_error(nl->ToString(error));
        }

        result->Append(*attribute);

        attribute->DeleteIfAllowed();
      }
    }

    correct = true;

    return Word(result);
  }
  catch (const std::exception &e)
  {
    errorInfo = simpleMessage(e.what());

    correct = false;

    return Word();
  }
}

ListExpr AttrArrayTC::Out(ListExpr typeExpr, Word value)
{
  AttrArrayTI typeInfo = AttrArrayTI(typeExpr);
  GenericAttrArray &instance = *(GenericAttrArray*)value.addr;

  GenericAttrArray::Iterator iterator(instance.GetIterator());

  if (iterator.IsValid())
  {
    const GenericAttrArray::PInfo &pInfo = instance.GetInfo();

    AttributeOutFunction outFunction(am->OutObj(pInfo->attributeAlgebraId,
                                                pInfo->attributeTypeId),
                                     typeInfo.GetAttributeType());

    const ListExpr attributeValues =
      nl->OneElemList(outFunction(iterator.GetAttribute()));

    ListExpr attributeValuesEnd = attributeValues;

    while (iterator.MoveToNext())
    {
      Append(attributeValuesEnd, outFunction(iterator.GetAttribute()));
    }

    return attributeValues;
  }
  else
  {
    return nl->Empty();
  }
}

Word AttrArrayTC::Create(const ListExpr typeExpr)
{
  AttrArrayTI typeInfo = AttrArrayTI(typeExpr);

  SmiFileId flobFileId =
    SecondoSystem::GetCatalog()->GetFlobFile()->GetFileId();

  return Word(new GenericAttrArray(typeInfo.GetPInfo(), flobFileId));
}

void AttrArrayTC::Delete(const ListExpr, Word &value)
{
  GenericAttrArray &instance = *(GenericAttrArray*)value.addr;

  instance.DeleteRecords();
  instance.DecRef();

  value.addr = NULL;
}

bool AttrArrayTC::Open(SmiRecord &valueRecord, size_t &offset,
                       const ListExpr typeExpr, Word &value)
{
  try
  {
    SmiReader source = SmiReader(valueRecord, offset);

    AttrArrayTI typeInfo = AttrArrayTI(typeExpr);

    GenericAttrArray *result = new GenericAttrArray(typeInfo.GetPInfo(),
                                                    source);

    offset = source.GetPosition();

    value = Word(result);
    return true;
  }
  catch (const std::exception &e)
  {
    value = Word();
    return false;
  }
}

bool AttrArrayTC::Save(SmiRecord &valueRecord, size_t &offset, const ListExpr,
                       Word &value)
{
  try
  {
    SmiWriter target = SmiWriter(valueRecord, offset);

    ((GenericAttrArray*)value.addr)->Save(target);

    offset = target.GetPosition();

    return true;
  }
  catch (const std::exception &e)
  {
    return false;
  }
}

void AttrArrayTC::Close(const ListExpr, Word &value)
{
  ((GenericAttrArray*)value.addr)->DecRef();

  value.addr = NULL;
}

void *AttrArrayTC::Cast(void *addr)
{
  return GenericAttrArray::Cast(addr);
}

int AttrArrayTC::SizeOf()
{
  return sizeof(GenericAttrArray);
}

Word AttrArrayTC::Clone(const ListExpr, const Word &value)
{
  GenericAttrArray &instance = *(GenericAttrArray*)value.addr;

  SmiFileId flobFileId =
    SecondoSystem::GetCatalog()->GetFlobFile()->GetFileId();

  GenericAttrArray *result = new GenericAttrArray(instance.GetInfo(),
                                                  flobFileId);

  GenericAttrArray::Iterator iterator = instance.GetIterator();
  if (iterator.IsValid())
  {
    do
    {
      result->Append(iterator.GetAttribute());
    }
    while (iterator.MoveToNext());
  }

  return Word(result);
}

AttrArrayTC::AttrArrayTC() :
  TypeConstructor(name, TypeProperty, Out, In, NULL, NULL, Create, Delete,
                  Open, Save, Close, Clone, Cast, SizeOf, CheckType)
{
}