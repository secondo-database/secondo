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

#include "GenericAttrArray.h"
#include "GenericSpatialAttrArray.h"
#include "Symbols.h"
#include "TypeConstructor.h"

//AttrArray---------------------------------------------------------------------

AttrArray::~AttrArray()
{
}

ArrayAttribute AttrArray::GetAt(size_t row) const
{
  return ArrayAttribute(this, row);
}

ArrayAttribute AttrArray::operator[](size_t row) const
{
  return ArrayAttribute(this, row);
}

void AttrArray::Append(const ArrayAttribute &value)
{
  Append(*value.m_block, value.m_row);
}

int AttrArray::Compare(size_t row, const ArrayAttribute &value) const
{
  return Compare(row, *value.m_block, value.m_row);
}

int AttrArray::Equals(size_t rowA, const AttrArray &blockB, size_t rowB) const
{
  return Compare(rowA, blockB, rowB) == 0;
}

int AttrArray::Equals(size_t row, Attribute &value) const
{
  return Compare(row, value) == 0;
}

int AttrArray::Equals(size_t row, const ArrayAttribute &value) const
{
  return Equals(row, *value.m_block, value.m_row);
}

void AttrArray::DeleteRecords()
{
}

void AttrArray::CloseFiles()
{
}

AttrArrayIterator AttrArray::GetIterator() const
{
  return AttrArrayIterator(this);
}

//ArrayAttribute----------------------------------------------------------------

ArrayAttribute::ArrayAttribute()
{
}

ArrayAttribute::ArrayAttribute(const AttrArray *block, size_t row) :
  m_block(block),
  m_row(row)
{
}

const AttrArray *ArrayAttribute::GetBlock() const
{
  return m_block;
}

size_t ArrayAttribute::GetRow() const
{
  return m_row;
}

int ArrayAttribute::Compare(const AttrArray &block, size_t row) const
{
  return m_block->Compare(m_row, block, row);
}

int ArrayAttribute::Compare(const ArrayAttribute &value) const
{
  return m_block->Compare(m_row, *value.m_block, value.m_row);
}

int ArrayAttribute::Compare(size_t row, Attribute &value) const
{
  return m_block->Compare(m_row, value);
}

bool ArrayAttribute::operator < (const ArrayAttribute& value)
{
  return Compare(value) < 0;
}

bool ArrayAttribute::operator <= (const ArrayAttribute& value)
{
  return Compare(value) <= 0;
}

bool ArrayAttribute::operator > (const ArrayAttribute& value)
{
  return Compare(value) > 0;
}

bool ArrayAttribute::operator >= (const ArrayAttribute& value)
{
  return Compare(value) >= 0;
}

int ArrayAttribute::ArrayAttribute::Equals(const AttrArray &block,
                                           size_t row) const
{
  return m_block->Compare(m_row, block, row);
}

int ArrayAttribute::Equals(const ArrayAttribute &value) const
{
  return m_block->Compare(m_row, *value.m_block, value.m_row);
}

int ArrayAttribute::Equals(Attribute &value) const
{
  return m_block->Compare(m_row, value);
}

bool ArrayAttribute::operator == (const ArrayAttribute& value)
{
  return Equals(value);
}

bool ArrayAttribute::operator != (const ArrayAttribute& value)
{
  return !Equals(value);
}

size_t ArrayAttribute::GetHash() const
{
  return m_block->GetHash(m_row);
}

ListExpr ArrayAttribute::GetListExpr() const
{
  return m_block->GetListExpr(m_row);
}

//AttrArrayIterator-------------------------------------------------------------

AttrArrayIterator::AttrArrayIterator() :
  m_count(0),
  m_current(NULL, 0)
{
}

AttrArrayIterator::AttrArrayIterator(const AttrArray *instance) :
  m_instance(instance),
  m_count(instance != NULL ? instance->GetCount() : 0),
  m_current(instance, 0)
{
}

bool AttrArrayIterator::IsValid() const
{
  return m_current.m_row < m_count;
}

bool AttrArrayIterator::MoveToNext()
{
  if (m_current.m_row < m_count)
  {
    return ++m_current.m_row < m_count;
  }

  return false;
}

ArrayAttribute &AttrArrayIterator::GetAttribute()
{
  return m_current;
}

//AttrArrayCatalog-----------------------------------------------------

AttrArrayCatalog &AttrArrayCatalog::GetInstance()
{
  return instance;
}

class GenericAttrArrayFactory : public AttrArrayFactory
{
public:
  GenericAttrArrayFactory(ListExpr attributeType) :
    m_info(GenericAttrArray::Info(attributeType))
  {
  }

  virtual AttrArray *Create(SmiFileId fileId)
  {
    return new GenericAttrArray(m_info, fileId);
  }

  virtual AttrArray *Load(Reader &source)
  {
    return new GenericAttrArray(m_info, source);
  }

protected:
  GenericAttrArray::PInfo m_info;
};

template<int dim>
class GenericSpatialAttrArrayFactory : public GenericAttrArrayFactory
{
public:
  GenericSpatialAttrArrayFactory(ListExpr attributeType) :
    GenericAttrArrayFactory(attributeType)
  {
  }

  virtual AttrArray *Create(SmiFileId fileId)
  {
    return new GenericSpatialAttrArray<dim>(m_info, fileId);
  }

  virtual AttrArray *Load(Reader &source)
  {
    return new GenericSpatialAttrArray<dim>(m_info, source);
  }
};

AttrArrayFactory *AttrArrayCatalog::CreateFactory(ListExpr attributeType)
{
  ListExpr typeExpr = attributeType;

  TypeConstructor *typeConstructor;

  while (true)
  {
    if (nl->IsAtom(typeExpr) || nl->IsEmpty(typeExpr))
    {
      return NULL;
    }

    const ListExpr first = nl->First(typeExpr);

    if (!nl->HasLength(typeExpr, 2) || !nl->IsNodeType(IntType, first))
    {
      typeExpr = first;
    }
    else
    {
      const ListExpr second = nl->Second(typeExpr);

      if (!nl->IsNodeType(IntType, second))
      {
        return NULL;
      }

      typeConstructor = am->GetTC(nl->IntValue(first), nl->IntValue(second));
      break;
    }
  }

  FactoryMap::EqualRangeIterator iterator =
    m_registrations.Get(typeConstructor->Name());

  if (iterator.IsValid())
  {
    do
    {
      AttrArrayRegistration &registration = *iterator.GetValue();

      if (registration.CheckAttributeType(attributeType))
      {
        return registration.Create(attributeType);
      }
    }
    while (iterator.MoveToNext());
  }

  if (typeConstructor->MemberOf(Kind::SPATIAL8D()))
  {
    return new GenericSpatialAttrArrayFactory<8>(attributeType);
  }
  else if (typeConstructor->MemberOf(Kind::SPATIAL4D()))
  {
    return new GenericSpatialAttrArrayFactory<4>(attributeType);
  }
  else if (typeConstructor->MemberOf(Kind::SPATIAL3D()))
  {
    return new GenericSpatialAttrArrayFactory<3>(attributeType);
  }
  else if (typeConstructor->MemberOf(Kind::SPATIAL2D()))
  {
    return new GenericSpatialAttrArrayFactory<2>(attributeType);
  }
  else if (typeConstructor->MemberOf(Kind::SPATIAL1D()))
  {
    return new GenericSpatialAttrArrayFactory<1>(attributeType);
  }

  return new GenericAttrArrayFactory(attributeType);
}

void AttrArrayCatalog::Register(AttrArrayRegistration *registration)
{
  m_registrations.Add(registration->GetAttributeTypeName(), registration);
}

AttrArrayCatalog AttrArrayCatalog::instance = AttrArrayCatalog();

size_t AttrArrayCatalog::HashString(const std::string &value)
{
  size_t hash = 5381;

  const size_t size = value.size();
  for (size_t i = 0; i < size; ++i)
  {
    hash = ((hash << 5) + hash) + value[i];
  }

  return hash;
}

int AttrArrayCatalog::CompareString(const std::string &a,
                                             const std::string &b)
{
  return a.compare(b);
}

AttrArrayCatalog::AttrArrayCatalog() :
  m_registrations(100)
{
}