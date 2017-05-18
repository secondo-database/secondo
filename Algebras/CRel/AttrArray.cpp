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

#include "AttrArray.h"

#include "FTextAlgebra.h"
#include "GAttrArrayTC.h"
#include "GSpatialAttrArrayTC.h"
#include "IntsTC.h"
#include "LinesTC.h"
#include "LogMsg.h"
#include "LongInt.h"
#include "LongIntsTC.h"
#include <map>
#include "RealsTC.h"
#include "SecondoSystem.h"
#include "SmiUtils.h"
#include "SpatialAlgebra.h"
#include "StandardTypes.h"
#include "StringsTC.h"
#include "Symbols.h"
#include "TextsTC.h"
#include "TypeConstructor.h"
#include "TypeUtils.h"

using namespace CRelAlgebra;

using std::string;
using std::map;

extern CMsg cmsg;
extern NestedList *nl;

//AttrArray---------------------------------------------------------------------

AttrArrayTypeConstructor *AttrArray::GetTypeConstructor(ListExpr type,
                                                        bool checkKind)
{
  TypeConstructor *typeConstructor = CRelAlgebra::GetTypeConstructor(type);

  if (typeConstructor != nullptr &&
      (!checkKind || typeConstructor->MemberOf(Kind::ATTRARRAY())))
  {
    return (AttrArrayTypeConstructor*)typeConstructor;
  }

  return nullptr;
}

//AttrArrayManager--------------------------------------------------------------

AttrArrayManager::AttrArrayManager() :
  m_refCount(1)
{
}

AttrArrayManager::~AttrArrayManager()
{
}

void AttrArrayManager::IncRef() const
{
  ++m_refCount;
}

void AttrArrayManager::DecRef() const
{
  if (--m_refCount == 0)
  {
    delete this;
  }
}

size_t AttrArrayManager::GetRefCount() const
{
  return m_refCount;
}

//AttrArrayTypeConstructor------------------------------------------------------

/*
The default ATTRARRAY types are determined by searching a map for the passed
attribute type's name.

If no default type is found we return a corresponding generic ATTRARRAY type.

Because Secondo can't stand static ~ListExpr~s we store functions.

*/
ListExpr AttrArrayTypeConstructor::GetDefaultAttrArrayType(
  ListExpr attributeType, bool numeric)
{
  static map<string, ListExpr(*)()> types =
  {
    { CcInt::BasicType(), [](){ return IntsTI(false).GetTypeExpr(); } },
    { LongInt::BasicType(), [](){ return LongIntsTI(false).GetTypeExpr(); } },
    { CcReal::BasicType(), [](){ return RealsTI(false).GetTypeExpr(); } },
    { CcString::BasicType(), [](){ return StringsTI(false).GetTypeExpr(); } },
    { FText::BasicType(), [](){ return TextsTI(false).GetTypeExpr(); } },
    { Line::BasicType(), [](){ return LinesTI(false).GetTypeExpr(); } }
  };

  TypeConstructor *typeConstructor = GetTypeConstructor(attributeType);

  if (typeConstructor == nullptr)
  {
    return nl->Empty();
  }

  map<string, ListExpr(*)()>::iterator result =
    types.find(typeConstructor->Name());

  if (result != types.end())
  {
    const ListExpr type = result->second();

    return numeric ? SecondoSystem::GetCatalog()->NumericType(type) : type;
  }

  if (typeConstructor->MemberOf(Kind::SPATIAL1D()))
  {
    GSpatialAttrArrayTI<1> genericTypeInfo = GSpatialAttrArrayTI<1>(numeric);

    genericTypeInfo.SetAttributeType(attributeType);

    return genericTypeInfo.GetTypeExpr();
  }

  if (typeConstructor->MemberOf(Kind::SPATIAL2D()))
  {
    GSpatialAttrArrayTI<2> genericTypeInfo = GSpatialAttrArrayTI<2>(numeric);

    genericTypeInfo.SetAttributeType(attributeType);

    return genericTypeInfo.GetTypeExpr();
  }

  if (typeConstructor->MemberOf(Kind::SPATIAL3D()))
  {
    GSpatialAttrArrayTI<3> genericTypeInfo = GSpatialAttrArrayTI<3>(numeric);

    genericTypeInfo.SetAttributeType(attributeType);

    return genericTypeInfo.GetTypeExpr();
  }

  if (typeConstructor->MemberOf(Kind::SPATIAL4D()))
  {
    GSpatialAttrArrayTI<4> genericTypeInfo = GSpatialAttrArrayTI<4>(numeric);

    genericTypeInfo.SetAttributeType(attributeType);

    return genericTypeInfo.GetTypeExpr();
  }

  if (typeConstructor->MemberOf(Kind::SPATIAL8D()))
  {
    GSpatialAttrArrayTI<8> genericTypeInfo = GSpatialAttrArrayTI<8>(numeric);

    genericTypeInfo.SetAttributeType(attributeType);

    return genericTypeInfo.GetTypeExpr();
  }

  GAttrArrayTI genericTypeInfo = GAttrArrayTI(numeric);

  genericTypeInfo.SetAttributeType(attributeType);

  return genericTypeInfo.GetTypeExpr();
}

/*
We use all possible default functions.

*/
AttrArrayTypeConstructor::AttrArrayTypeConstructor(const std::string& name,
  TypeProperty typeProperty, TypeCheckFunction typeCheck,
  AttrArrayTypeFunction attributeType, AttrArrayManagerFunction manager) :
  AttrArrayTypeConstructor(name, typeProperty, DefaultOut, DefaultIn,
                           DefaultCreate, DefaultDelete, DefaultOpen,
                           DefaultSave, DefaultClose, DefaultClone, DefaultCast,
                           DefaultSizeOf, typeCheck, attributeType, manager)
{
}

AttrArrayTypeConstructor::AttrArrayTypeConstructor(const string& name,
  TypeProperty typeProperty, OutObject out, InObject in, ObjectCreation create,
  ObjectDeletion del, ObjectOpen open, ObjectSave save, ObjectClose close,
  ObjectClone clone, ObjectCast cast, ObjectSizeof sizeOf,
  TypeCheckFunction typeCheck, AttrArrayTypeFunction attributeType,
  AttrArrayManagerFunction manager) :
  TypeConstructor(name, typeProperty, out, in, nullptr, nullptr, create, del,
                  open, save, close, clone, cast, sizeOf, typeCheck),
  m_getAttributeType(attributeType),
  m_createManager(manager)
{
  AssociateKind(Kind::ATTRARRAY());
}

ListExpr AttrArrayTypeConstructor::GetAttributeType(ListExpr typeExpr,
                                                    bool numeric)
{
  return m_getAttributeType(typeExpr, numeric);
}

AttrArrayManager *AttrArrayTypeConstructor::CreateManager(
  ListExpr attributeType)
{
  return m_createManager(attributeType);
}

Word AttrArrayTypeConstructor::DefaultIn(ListExpr typeExpr, ListExpr value,
                                         int errorPos, ListExpr &errorInfo,
                                         bool &correct)
{
  try
  {
    AttrArrayTypeConstructor *typeConstructor =
      (AttrArrayTypeConstructor*)GetTypeConstructor(typeExpr);

    const SmiFileId fileId =
      SecondoSystem::GetCatalog()->GetFlobFile()->GetFileId();

    const ListExpr attrType = typeConstructor->GetAttributeType(typeExpr,
                                                                true);

    const InObject attrIn = GetInFunction(attrType);

    AttrArrayManager *manager = typeConstructor->CreateManager(attrType);

    AttrArray *result = manager->Create(fileId);

    manager->DecRef();

    ListExpr attributeValues = value;

    if (nl->IsAtom(attributeValues))
    {
      cmsg.inFunError(nl->ToString(typeExpr) +
                      ": Expected list of attribute values.");

      correct = false;

      result->DecRef();

      return Word();
    }

    std::string error;

    size_t row = 0;

    while (!nl->IsEmpty(attributeValues))
    {
      Word attrInResult = attrIn(attrType, nl->First(attributeValues),
                                  row++, errorInfo, correct);

      if (!correct)
      {
        cmsg.inFunError("Attribute InFunction failed: "+
                        nl->ToString(errorInfo));

        result->DecRef();

        return Word();
      }

      Attribute *attribute = (Attribute*)attrInResult.addr;

      result->Append(*attribute);

      attribute->DeleteIfAllowed();

      attributeValues = nl->Rest(attributeValues);
    }

    correct = true;

    return Word(result);
  }
  catch (const std::exception &e)
  {
    cmsg.inFunError(nl->ToString(typeExpr) + ": " + e.what());

    correct = false;

    return Word();
  }
}

ListExpr AttrArrayTypeConstructor::DefaultOut(ListExpr typeExpr, Word value)
{
  AttrArrayTypeConstructor *typeConstructor =
    (AttrArrayTypeConstructor*)GetTypeConstructor(typeExpr);

  const ListExpr attrType = typeConstructor->GetAttributeType(typeExpr,
                                                              true);

  const OutObject attrOut = GetOutFunction(attrType);

  AttrArray &instance = *(AttrArray*)value.addr;

  ListExpr attributeValues = nl->OneElemList(nl->Empty()),
    attributeValuesEnd = attributeValues;

  for (AttrArrayEntry &entry : instance.GetFilter())
  {
    Attribute *attr = entry.GetAttribute();

    attributeValuesEnd = nl->Append(attributeValuesEnd, attrOut(attrType,
                                                                attr));

    attr->DeleteIfAllowed();
  }

  return nl->Rest(attributeValues);
}

Word AttrArrayTypeConstructor::DefaultCreate(const ListExpr typeExpr)
{
  const SmiFileId fileId =
    SecondoSystem::GetCatalog()->GetFlobFile()->GetFileId();

  AttrArrayTypeConstructor *typeConstructor =
    (AttrArrayTypeConstructor*)GetTypeConstructor(typeExpr);

  const ListExpr attrType = typeConstructor->GetAttributeType(typeExpr,
                                                              true);

  AttrArrayManager *manager = typeConstructor->CreateManager(attrType);

  AttrArray *result = manager->Create(fileId);

  manager->DecRef();

  return result;
}

void AttrArrayTypeConstructor::DefaultDelete(const ListExpr typeExpr,
                                             Word &value)
{
  AttrArray &instance = *(AttrArray*)value.addr;

  instance.DeleteRecords();
  instance.DecRef();

  value.addr = nullptr;
}

bool AttrArrayTypeConstructor::DefaultOpen(SmiRecord &valueRecord,
                                           size_t &offset,
                                           const ListExpr typeExpr, Word &value)
{
  try
  {
    SmiReader source = SmiReader(valueRecord, offset);

    AttrArrayTypeConstructor *typeConstructor =
      (AttrArrayTypeConstructor*)GetTypeConstructor(typeExpr);

    const ListExpr attrType = typeConstructor->GetAttributeType(typeExpr, true);

    AttrArrayManager *manager = typeConstructor->CreateManager(attrType);

    AttrArray *result = manager->Load(source);

    manager->DecRef();

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

bool AttrArrayTypeConstructor::DefaultSave(SmiRecord &valueRecord,
                                           size_t &offset,
                                           const ListExpr typeExpr, Word &value)
{
  try
  {
    SmiWriter target = SmiWriter(valueRecord, offset);

    ((AttrArray*)value.addr)->Save(target, true);

    offset = target.GetPosition();

    return true;
  }
  catch (const std::exception &e)
  {
    return false;
  }
}

void AttrArrayTypeConstructor::DefaultClose(const ListExpr typeExpr,
                                            Word &value)
{
  ((AttrArray*)value.addr)->DecRef();

  value.addr = nullptr;
}

void *AttrArrayTypeConstructor::DefaultCast(void *addr)
{
  return nullptr;
}

int AttrArrayTypeConstructor::DefaultSizeOf()
{
  return 0;
}

Word AttrArrayTypeConstructor::DefaultClone(const ListExpr typeExpr,
                                            const Word &value)
{
  AttrArray &instance = *(AttrArray*)value.addr;

  AttrArray *result = (AttrArray*)DefaultCreate(typeExpr).addr;

  for (AttrArrayEntry &entry : instance.GetFilter())
  {
    result->Append(entry);
  }

  return Word(result);
}