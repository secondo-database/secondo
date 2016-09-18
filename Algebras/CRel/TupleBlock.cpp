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

#include "TupleBlock.h"

#include "AlgebraManager.h"
#include "ListUtils.h"
#include "LogMsg.h"
#include "RelationAlgebra.h"
#include "TupleBlockType.h"
#include "QueryProcessor.h"

using namespace listutils;

using std::string;

extern NestedList *nl;
extern QueryProcessor *qp;
extern AlgebraManager *am;

TypeConstructor TupleBlock::typeConstructor(TupleBlock::GetBasicType(), Prop,
                                            Out, In,
                                            SaveToList, RestoreFromList,
                                            Create, Delete,
                                            Open, Save, Close,
                                            Clone, Cast, SizeOf, Check);

TypeConstructor &TupleBlock::GetTypeConstructor()
{
  return typeConstructor;
}

string TupleBlock::GetBasicType()
{
  return "tblock";
}

TupleBlock::~TupleBlock()
{
  int attributeCount = m_type->GetAttributeCount();
  for (int i = 0; i < attributeCount; i++)
  {
    delete[] m_attributes[i];
  }

  delete[] m_attributes;
}

ListExpr TupleBlock::Prop()
{
  return nl->TwoElemList(
      nl->FourElemList(nl->StringAtom("Signature"),
                       nl->StringAtom("Example Type List"),
                       nl->StringAtom("List Rep"),
                       nl->StringAtom("Example List")),
      nl->FourElemList(nl->StringAtom(""),
                       nl->StringAtom(""),
                       nl->TextAtom(""),
                       nl->TextAtom("")));
}

bool TupleBlock::Check(ListExpr type, ListExpr &errorInfo)
{
  if (nl->ListLength(type) != 3 ||
      !isSymbol(nl->First(type), TupleBlock::GetBasicType()) ||
      !isAttrList(nl->Third(type)))
  {
    return false;
  }

  ListExpr second = nl->Second(type);
  if (nl->AtomType(second) != IntType || nl->IntValue(second) <= 0)
  {
    return false;
  }

  if (!checkAttrListForNamingConventions(nl->Third(type)))
  {
    cmsg.typeError("Attribute names do not fit Secondo's name conventions\n"
                   "(an attribute name must start with a capital letter.)\n");
    return false;
  }

  return true;
}

Word TupleBlock::In(ListExpr typeInfo, ListExpr value,
                    int errorPos, ListExpr &errorInfo,
                    bool &correct)
{
  correct = true;

  ListExpr attributeTypeInfosLE = nl->Third(typeInfo);
  int attributeCount = nl->ListLength(attributeTypeInfosLE);
  ListExpr* attributeTypeInfos = new ListExpr[attributeCount];

  for (int i = 0; i < attributeCount; i++)
  {
    attributeTypeInfos[i] = nl->First(attributeTypeInfosLE);
    attributeTypeInfosLE = nl->Rest(attributeTypeInfosLE);
  }

  TupleBlock *instance = new TupleBlock();
  instance->m_type = PTupleBlockType(attributeCount, attributeTypeInfos,
                                     nl->IntValue(nl->Second(typeInfo)));
  instance->m_attributes = new Address *[attributeCount];

  InObject* attributeInFunctions = new InObject[attributeCount];
  int capacity = instance->m_type->GetCapacity();
  for (int i = 0; i < attributeCount; i++)
  {
    const AttributeType &attributeType = instance->m_type->GetAttributeType(i);
    attributeInFunctions[i] = am->InObj(attributeType.algId, 
                                        attributeType.typeId);

    instance->m_attributes[i] = new Address[capacity];
  }

  int tupleCount = 0;
  ListExpr tupleValues = value;
  while (!nl->IsEmpty(tupleValues))
  {
    ListExpr attributeValues = nl->First(tupleValues);
    for (int i = 0; i < attributeCount; i++)
    {
      bool valueCorrect;
      Word attr = attributeInFunctions[i](attributeTypeInfos[i], 
                                          nl->First(attributeValues),
                                          i, errorInfo, valueCorrect);

      attributeValues = nl->Rest(attributeValues);

      if (valueCorrect)
      {
        correct = true;

        instance->m_attributes[i][tupleCount] = attr.addr;
      }
      else
      {
        correct = false;
      }
    }

    tupleValues = nl->Rest(tupleValues);
    tupleCount++;
  }

  delete[] attributeTypeInfos;
  delete[] attributeInFunctions;

  return Word((Address)instance);
}

ListExpr TupleBlock::Out(ListExpr typeInfo, Word value)
{
  return nl->Empty();
}

ListExpr TupleBlock::SaveToList(ListExpr typeInfo, Word value)
{
  return nl->Empty();
}

Word TupleBlock::RestoreFromList(ListExpr typeInfo, ListExpr value,
                                 int errorPos, ListExpr &errorInfo,
                                 bool &correct)
{
  return Word((Address)NULL);
}

Word TupleBlock::Create(const ListExpr typeInfo)
{
  return Word((Address)NULL);
}

Word TupleBlock::Clone(const ListExpr typeInfo, const Word &w)
{
  return Word((Address)NULL);
}

bool TupleBlock::Save(SmiRecord &valueRecord, size_t &offset,
                      const ListExpr typeInfo, Word &value)
{
  return false;
}

void TupleBlock::Delete(const ListExpr typeInfo, Word &w)
{
}

bool TupleBlock::Open(SmiRecord &valueRecord, size_t &offset,
                      const ListExpr typeInfo, Word &value)
{
  return false;
}

void TupleBlock::Close(const ListExpr typeInfo, Word &w)
{
}

void *TupleBlock::Cast(void *addr)
{
  return NULL;
}

int TupleBlock::SizeOf()
{
  return 0;
}

TupleBlock::TupleBlock() : m_type(NULL),
                           m_attributes(NULL)
{
}