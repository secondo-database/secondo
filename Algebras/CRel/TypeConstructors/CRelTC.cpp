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
#include "CRelTC.h"

#include "AlgebraTypes.h"
#include "AlgebraManager.h"
#include <exception>
#include "LogMsg.h"
#include "Shared.h"
#include "StringUtils.h"
#include "TBlock.h"
#include "TBlockTC.h"
#include "TypeUtils.h"

using namespace CRelAlgebra;

using std::exception;
using std::string;
using stringutils::any2str;

extern CMsg cmsg;
extern NestedList *nl;
extern AlgebraManager *am;

//CRelTI------------------------------------------------------------------------

bool CRelTI::Check(ListExpr typeExpr)
{
  string error;

  return Check(typeExpr, error);
}

bool CRelTI::Check(ListExpr typeExpr, string &error)
{
  if (!nl->HasLength(typeExpr, 2))
  {
    error = "TypeInfo's length != 2.";
    return false;
  }

  if (nl->IsEqual(nl->First(typeExpr), Symbols::STREAM()))
  {
    typeExpr = nl->Second(typeExpr);
  }

  if (!nl->IsEqual(nl->First(typeExpr), CRelTC::name))
  {
    error = "TypeInfo's first element != " + CRelTC::name + ".";
    return false;
  }

  ListExpr parameters = nl->Second(typeExpr);
  if (nl->IsAtom(parameters) || !nl->HasLength(parameters, 2))
  {
    error = "TypeInfo's second element isn't a three element list.";
    return false;
  }

  ListExpr cacheSizePara = nl->First(parameters);
  long cacheSize;

  if (!nl->IsNodeType(IntType, cacheSizePara) ||
      (cacheSize = nl->IntValue(cacheSizePara)) < 1)
  {
    error = "TypeInfo's first parameter (cache size) is not an int > 0.";
    return false;
  }

  return TBlockTI::Check(nl->Second(parameters), error);
}

CRelTI::CRelTI(bool numeric) :
  TBlockTI(numeric)
{
}

CRelTI::CRelTI(const TBlockTI &info, size_t cacheSize) :
  TBlockTI(info),
  m_cacheSize(cacheSize)
{
}

CRelTI::CRelTI(ListExpr typeExpr, bool numeric) :
  TBlockTI(numeric)
{
  if (nl->IsEqual(nl->First(typeExpr), Symbols::STREAM()))
  {
    *this = CRelTI(nl->Second(typeExpr), numeric);
    return;
  }

  const ListExpr parameters = nl->Second(typeExpr);

  *this = CRelTI(TBlockTI(nl->Second(parameters), numeric),
                 nl->IntValue(nl->First(parameters)));
}

size_t CRelTI::GetCacheSize() const
{
  return m_cacheSize;
}

void CRelTI::SetCacheSize(size_t value)
{
  m_cacheSize = value;
}

ListExpr CRelTI::GetTypeExpr() const
{
  if (IsNumeric())
  {
    return nl->TwoElemList(GetNumericType(CRelTC::name),
                           nl->TwoElemList(nl->IntAtom(m_cacheSize),
                                           TBlockTI::GetTypeExpr()));
  }
  else
  {
    return nl->TwoElemList(nl->SymbolAtom(CRelTC::name),
                           nl->TwoElemList(nl->IntAtom(m_cacheSize),
                                           TBlockTI::GetTypeExpr()));
  }
}

//CRelTC------------------------------------------------------------------------

const string CRelTC::name = "crel";

ListExpr CRelTC::TypeProperty()
{
  return ConstructorInfo(
    name,
    "int x TBLOCK -> " + name,
    "(" + name + " (1 (" + TBlockTC::name +
    " (1000 ((Name string)(Age: int))))))))",
    "((<attr1> ... <attrn>)*)",
    "((\"Myers\" 53)(\"Smith\" 21))",
    "The int parameter defines the maximum number of cached blocks.").list();
}

bool CRelTC::CheckType(ListExpr typeExpr, ListExpr &errorInfo)
{
  if (!CRelTI::Check(typeExpr))
  {
    return false;
  }

  return true;
}

Word CRelTC::In(ListExpr typeExpr, ListExpr value, int errorPos,
                ListExpr &errorInfo, bool &correct)
{
  correct = true;

  try
  {
    const CRelTI typeInfo = CRelTI(typeExpr, true);
    const PTBlockInfo blockInfo = typeInfo.GetBlockInfo();
    const size_t columnCount = blockInfo->columnCount;

    SharedArray<InObject> inFunctions(columnCount);
    SharedArray<ListExpr> attributeTypes(columnCount);

    for (size_t i = 0; i < columnCount; i++)
    {
      const ListExpr type = blockInfo->columnAttributeTypes[i];

      int algebraId,
        typeId;

      ResolveTypeOrThrow(type, algebraId, typeId);

      inFunctions[i] = am->InObj(algebraId, typeId);
      attributeTypes[i] = type;
    }

    CRel *relation = new CRel(blockInfo, typeInfo.GetDesiredBlockSize() *
                              CRelTI::blockSizeFactor, typeInfo.GetCacheSize());

    ListExpr tupleValues = value;

    if (nl->IsAtom(tupleValues))
    {
      correct = false;

      cmsg.inFunError(name + ": List of tuples expected.");

      relation->DeleteFiles();
      delete relation;

      return Word();

      /*
      errorInfo = nl->Append(errorInfo, nl->FourElemList(
        nl->IntAtom(ERR_SPECIFIC_FOR_TYPE_CONSTRUCTOR),
        nl->SymbolAtom(name), nl->IntAtom(0),
        nl->TextAtom("List of tuples expected.")));
      */
    }

    SharedArray<Attribute*> tuple(columnCount);

    size_t index = 0;

    while (!nl->IsEmpty(tupleValues))
    {
      ListExpr attributeValues = nl->First(tupleValues);

      if (nl->IsAtom(attributeValues))
      {
        correct = false;

        cmsg.inFunError(name + ": Invalid tuple at position " +
                        any2str(index / columnCount) + ".");

        relation->DeleteFiles();
        delete relation;

        return Word();
      }

      tupleValues = nl->Rest(tupleValues);

      for (size_t i = 0; i < columnCount; ++i)
      {
        if (nl->IsEmpty(attributeValues))
        {
          correct = false;

          cmsg.inFunError(name + ": Not enough values in tuple " +
                          any2str(index / columnCount) + '\n');

          relation->DeleteFiles();
          delete relation;

          return Word();
        }

        tuple[i] = (Attribute*)inFunctions[i](attributeTypes[i],
                                              nl->First(attributeValues), index,
                                              errorInfo, correct).addr;

        attributeValues = nl->Rest(attributeValues);

        if (!correct)
        {
          cmsg.inFunError(name + ": Value for attribute no. " + any2str(i) +
                          " in tuple no. " + any2str(index / columnCount) +
                          " is not valid.");

          relation->DeleteFiles();
          delete relation;

          return Word();
        }

        ++index;
      }

      relation->Append(tuple.GetPointer());

      for (size_t i = 0; i < columnCount; ++i)
      {
        tuple[i]->DeleteIfAllowed();
      }
    }

    return Word(relation);
  }
  catch (const exception &e)
  {
    cmsg.inFunError(e.what());

    correct = false;

    return Word();
  }
}

ListExpr CRelTC::Out(ListExpr typeExpr, Word value)
{
  const CRel &instance = *(CRel*)value.addr;

  const size_t columnCount = instance.GetColumnCount();

  OutObject *attrOuts = new OutObject[columnCount];

  const ListExpr *attrTypes = instance.GetBlockInfo()->columnAttributeTypes;

  for (size_t i = 0; i < columnCount; ++i)
  {
    attrOuts[i] = GetOutFunction(attrTypes[i]);
  }

  if (columnCount > 0)
  {
    const CRelTI typeInfo = CRelTI(typeExpr, true);

    const ListExpr tupleExprList = nl->OneElemList(nl->Empty());
      ListExpr tupleExprListEnd = tupleExprList;

    for (const TBlockEntry &tuple : instance)
    {
      const ListExpr tupleExpr = nl->OneElemList(nl->Empty());
      ListExpr tupleExprEnd = tupleExpr;

      for (size_t i = 0; i < columnCount; ++i)
      {
        Attribute *attr = tuple[i].GetAttribute();

        tupleExprEnd = nl->Append(tupleExprEnd, attrOuts[i](attrTypes[i],
                                                            attr));

        attr->DeleteIfAllowed();
      }

      tupleExprListEnd = nl->Append(tupleExprListEnd, nl->Rest(tupleExpr));
    }

    return nl->Rest(tupleExprList);
  }

  return nl->Empty();
}

Word CRelTC::Create(const ListExpr typeExpr)
{
  const CRelTI typeInfo = CRelTI(typeExpr, true);

  return Word(new CRel(typeInfo.GetBlockInfo(), typeInfo.GetDesiredBlockSize() *
                       CRelTI::blockSizeFactor, typeInfo.GetCacheSize()));
}

void CRelTC::Delete(const ListExpr, Word &value)
{
  CRel *instance = (CRel*)value.addr;

  instance->DeleteFiles();

  delete instance;

  value.addr = nullptr;
}

bool CRelTC::Open(SmiRecord &valueRecord, size_t &offset,
                  const ListExpr typeExpr, Word &value)
{
  try
  {
    const CRelTI typeInfo = CRelTI(typeExpr, true);

    SmiReader source = SmiReader(valueRecord, offset);

    value = Word(new CRel(typeInfo.GetBlockInfo(), typeInfo.GetCacheSize(),
                          source));

    offset = source.GetPosition();

    return true;
  }
  catch (const exception &e)
  {
    value = Word();

    return false;
  }
}

bool CRelTC::Save(SmiRecord &valueRecord, size_t &offset, const ListExpr,
                  Word &value)
{
  try
  {
    SmiWriter target = SmiWriter(valueRecord, offset);

    ((CRel*)value.addr)->Save(target);

    offset = target.GetPosition();

    return true;
  }
  catch (const std::exception &e)
  {
    return false;
  }
}

void CRelTC::Close(const ListExpr, Word &value)
{
  delete (CRel*)value.addr;

  value.addr = nullptr;
}

void *CRelTC::Cast(void *addr)
{
  return nullptr;
}

int CRelTC::SizeOf()
{
  return 0;
}

Word CRelTC::Clone(const ListExpr, const Word &value)
{
  CRel &instance = *(CRel*)value.addr,
    *clone = new CRel(instance.GetBlockInfo(), instance.GetDesiredBlockSize() *
                      CRelTI::blockSizeFactor, instance.GetCacheSize());

  for (const TBlockEntry &tuple : instance)
  {
    clone->Append(tuple);
  }

  return Word(clone);
}

CRelTC::CRelTC() :
  TypeConstructor(name, TypeProperty, Out, In, nullptr, nullptr, Create, Delete,
                  Open, Save, Close, Clone, Cast, SizeOf, CheckType)
{
}