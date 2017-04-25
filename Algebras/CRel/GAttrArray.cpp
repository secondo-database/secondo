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

#include "GAttrArray.h"

#include "AlgebraManager.h"
#include <algorithm>
#include <cstring>
#include "ListUtils.h"
#include "SecondoException.h"
#include "SmiUtils.h"
#include "TypeUtils.h"

using namespace CRelAlgebra;

using listutils::emptyErrorInfo;
using std::max;
using std::string;

extern AlgebraManager *am;

GAttrArrayInfo::GAttrArrayInfo()
{
}

GAttrArrayInfo::GAttrArrayInfo(ListExpr attributeTypeExpr) :
  attributeType(attributeTypeExpr)
{
  ResolveTypeOrThrow(attributeTypeExpr, attributeAlgebraId, attributeTypeId);

  TypeConstructor &typeConstructor = *am->GetTC(attributeAlgebraId,
                                                attributeTypeId);

  attributeSize =  typeConstructor.SizeOf();

  attributeInFunction = am->InObj(attributeAlgebraId, attributeTypeId);

  attributeOutFunction = am->OutObj(attributeAlgebraId, attributeTypeId);

  attributeCastFunction = am->Cast(attributeAlgebraId, attributeTypeId);

  attributeFLOBCount = max(typeConstructor.NumOfFLOBs(), 0);
}

GAttrArray::GAttrArray(const PGAttrArrayInfo &info, SmiFileId flobFileId) :
  m_header(0, flobFileId),
  m_info(info),
  m_capacity(0),
  m_size(0),
  m_attributeData(nullptr),
  m_attributeDataEnd(nullptr)
{
}

GAttrArray::GAttrArray(const PGAttrArrayInfo &info, Reader &source) :
  GAttrArray(info, source, source.ReadOrThrow<GAttrArrayHeader>())
{
}

GAttrArray::GAttrArray(const PGAttrArrayInfo &info, Reader &source,
                       const GAttrArrayHeader &header) :
  m_header(header),
  m_info(info),
  m_capacity(m_header.count),
  m_size(sizeof(GAttrArray) +
         (m_header.count * (info->attributeSize + sizeof(char)))),
  m_attributeData(m_header.count > 0 ?
                  new char[info->attributeSize * m_header.count] : nullptr),
  m_attributeDataEnd(m_header.count > 0 ?
    m_attributeData + (info->attributeSize * m_header.count) : nullptr)
{
  const size_t count = m_header.count;

  if (count > 0)
  {
    const size_t attributeSize = info->attributeSize;

    source.ReadOrThrow(m_attributeData, attributeSize * count);

    ObjectCast cast = m_info->attributeCastFunction;

    char *attribute = m_attributeData,
      *end = m_attributeDataEnd;

    while (attribute < end)
    {
      cast((Attribute*)attribute);

      attribute += attributeSize;
    }
  }
}

GAttrArray::~GAttrArray()
{
  const size_t count = m_header.count;
  char * const attributeData = m_attributeData;

  if (count > 0)
  {
    const GAttrArrayInfo &info = *m_info;

    const size_t flobCount = info.attributeFLOBCount,
      attributeSize = info.attributeSize;

    if (flobCount > 0)
    {
      char *currentAttributeData = attributeData,
           *attributeDataEnd = currentAttributeData + (count * attributeSize);

      while (currentAttributeData < attributeDataEnd)
      {
        Attribute *attribute = (Attribute*)currentAttributeData;

        for (size_t i = 0; i < flobCount; i++)
        {
          attribute->GetFLOB(i)->~Flob();
        }

        currentAttributeData += attributeSize;
      }
    }
  }

  if (attributeData != nullptr)
  {
    delete[] attributeData;
  }
}

const PGAttrArrayInfo &GAttrArray::GetInfo() const
{
  return m_info;
}

size_t GAttrArray::GetCount() const
{
  return m_header.count;
}

size_t GAttrArray::GetSize() const
{
  return m_size;
}

void GAttrArray::Save(Writer &target, bool includeHeader) const
{
  if (includeHeader)
  {
    target.WriteOrThrow(m_header);
  }

  const size_t count = m_header.count;

  if (count > 0)
  {
    target.WriteOrThrow(m_attributeData, count * m_info->attributeSize);
  }
}

void GAttrArray::DeleteRecords()
{
  /*
  Delete all ~Flob~ records.

  */

  const GAttrArrayInfo &info = *m_info;

  const size_t flobCount = info.attributeFLOBCount;

  if (m_header.flobFileId != 0 && flobCount > 0)
  {
    const size_t count = m_header.count;

    if (count > 0)
    {
      const size_t attributeSize = info.attributeSize;

      char *currentAttributeData = m_attributeData,
        *attributeDataEnd = m_attributeDataEnd;

      while (currentAttributeData < attributeDataEnd)
      {
        Attribute *attribute = (Attribute*)currentAttributeData;

        for (size_t i = 0; i < flobCount; i++)
        {
          attribute->GetFLOB(i)->destroy();
        }

        currentAttributeData += attributeSize;
      }
    }
  }
}

void GAttrArray::Append(const AttrArray &array, size_t row)
{
  Append(((GAttrArray&)array).GetAt(row));
}

void GAttrArray::Append(Attribute &value)
{
  const GAttrArrayInfo &info = *m_info;
  const size_t attributeSize = info.attributeSize;

  size_t capacity = m_capacity,
    count = m_header.count;

  char *attributeData,
    *attributeDataEnd;

  //resize?
  if (capacity == count)
  {
    //first element?
    if (capacity == 0)
    {
      capacity = 1;

      attributeData = m_attributeData = new char[attributeSize];

      attributeDataEnd = m_attributeDataEnd = attributeData;
    }
    else
    {
      const size_t newCapacity = capacity * 2;

      attributeData = new char[newCapacity * attributeSize];

      attributeDataEnd = m_attributeDataEnd =
        attributeData + (capacity * attributeSize);

      char *oldAttributeData = m_attributeData;

      //copy existing values
      memcpy(attributeData, oldAttributeData, capacity * attributeSize);

      delete[] oldAttributeData;

      capacity = newCapacity;

      m_attributeData = attributeData;
    }

    m_capacity = capacity;
  }
  else
  {
    attributeData = m_attributeData;
    attributeDataEnd = m_attributeDataEnd;
  }

  Attribute *attribute = (Attribute*)attributeDataEnd;

  //copy the ne attribute's data
  memcpy(attribute, &value, attributeSize);

  const size_t flobCount = info.attributeFLOBCount;

  //copy Flobs?
  if (flobCount > 0)
  {
    const SmiFileId flobFileId = m_header.flobFileId;

    if (flobFileId != 0)
    {
      for (size_t j = 0; j < flobCount; j++)
      {
        Flob &sourceFlob = *value.GetFLOB(j),
          *targetFlob = attribute->GetFLOB(j);

        new (targetFlob) Flob(0);

        sourceFlob.saveToFile(flobFileId, 0, *targetFlob);
      }
    }
    else
    {
      for (size_t j = 0; j < flobCount; j++)
      {
        Flob &sourceFlob = *value.GetFLOB(j),
          *targetFlob = attribute->GetFLOB(j);

        new (targetFlob) Flob(sourceFlob);
      }
    }
  }

  //decrese the stored attribute's ref count to 1
  while (attribute->NoRefs() > 1)
  {
    attribute->DeleteIfAllowed();
  }

  ++m_header.count;

  m_size += attributeSize;
  m_attributeDataEnd += attributeSize;
}

void GAttrArray::Remove()
{
  /*
  Remove last ~Attribute~ and delete it's copied ~Flob~s

  */
  const GAttrArrayInfo &info = *m_info;

  const size_t flobCount = info.attributeFLOBCount,
   count = m_header.count;

  if (count > 0 && flobCount > 0 && m_header.flobFileId != 0)
  {
    const size_t attributeSize = info.attributeSize;

    Attribute *attribute = (Attribute*)(m_attributeDataEnd -= attributeSize);

    for (size_t i = 0; i < flobCount; i++)
    {
      attribute->GetFLOB(i)->destroy();
    }

    --m_header.count;

    m_size -= attributeSize;
  }
}

void GAttrArray::Clear()
{
  if (m_header.count > 0)
  {
    DeleteRecords();

    m_header.count = 0;

    m_attributeDataEnd = m_attributeData;

    m_size = 0;
  }
}

Attribute &GAttrArray::GetAt(size_t row) const
{
  return *(Attribute*)(m_attributeData + (row * m_info->attributeSize));
}

Attribute &GAttrArray::operator[](size_t row) const
{
  return *(Attribute*)(m_attributeData + (row * m_info->attributeSize));
}

bool GAttrArray::IsDefined(size_t row) const
{
  return GetAt(row).IsDefined();
}

int GAttrArray::Compare(size_t rowA, const AttrArray &arrayB,
                              size_t rowB) const
{
  return GetAt(rowA).Compare(&((GAttrArray&)arrayB).GetAt(rowB));
}

int GAttrArray::Compare(size_t row, Attribute &value) const
{
  return GetAt(row).Compare(&value);
}

bool GAttrArray::Equals(size_t rowA, const AttrArray &arrayB,
                              size_t rowB) const
{
  return GetAt(rowA).Equal(&((GAttrArray&)arrayB).GetAt(rowB));
}

bool GAttrArray::Equals(size_t row, Attribute &value) const
{
  return GetAt(row).Equal(&value);
}

size_t GAttrArray::GetHash(size_t row) const
{
  return GetAt(row).HashValue();
}

Attribute *GAttrArray::GetAttribute(size_t row, bool clone) const
{
  return clone ? GetAt(row).Clone() : GetAt(row).Copy();
}

GAttrArrayIterator GAttrArray::GetIterator() const
{
  return GAttrArrayIterator(*this);
}

GAttrArrayIterator GAttrArray::begin() const
{
  return GetIterator();
}

GAttrArrayIterator GAttrArray::end() const
{
  return GAttrArrayIterator();
}

//GAttrArrayHeader-------------------------------------------

GAttrArrayHeader::GAttrArrayHeader()
{
}

GAttrArrayHeader::GAttrArrayHeader(size_t count, SmiFileId flobFileId) :
  count(count),
  flobFileId(flobFileId)
{
}

GAttrArrayHeader::GAttrArrayHeader(const AttrArrayHeader &header) :
  count(header.count),
  flobFileId(header.flobFileId)
{
}

//GSpatialAttrArray<dim>--------------------------------------------------------

template<int dim>
GSpatialAttrArray<dim>::GSpatialAttrArray(const PGAttrArrayInfo &info) :
  m_array(info)
{
}

template<int dim>
GSpatialAttrArray<dim>::GSpatialAttrArray(const PGAttrArrayInfo &info,
                                          SmiFileId flobFileId) :
  m_array(info, flobFileId)
{
}

template<int dim>
GSpatialAttrArray<dim>::GSpatialAttrArray(const PGAttrArrayInfo &info,
                                          Reader &source) :
  m_array(info, source)
{
}

template<int dim>
GSpatialAttrArray<dim>::GSpatialAttrArray(const PGAttrArrayInfo &info,
                                          Reader &source,
                                          const GAttrArrayHeader &header) :
  m_array(info, source, header)
{
}

template<int dim>
size_t GSpatialAttrArray<dim>::GetCount() const
{
  return m_array.GetCount();
}

template<int dim>
size_t GSpatialAttrArray<dim>::GetSize() const
{
  return m_array.GetSize();
}

template<int dim>
void GSpatialAttrArray<dim>::Save(Writer &target, bool includeHeader) const
{
  m_array.Save(target, includeHeader);
}

template<int dim>
void GSpatialAttrArray<dim>::DeleteRecords()
{
  m_array.DeleteRecords();
}

template<int dim>
void GSpatialAttrArray<dim>::Append(const AttrArray &array, size_t row)
{
  m_array.Append(((GSpatialAttrArray<dim>&)array).m_array, row);
}

template<int dim>
void GSpatialAttrArray<dim>::Append(Attribute &value)
{
  m_array.Append(value);
}

template<int dim>
void GSpatialAttrArray<dim>::Remove()
{
  m_array.Remove();
}

template<int dim>
void GSpatialAttrArray<dim>::Clear()
{
  m_array.Clear();
}

template<int dim>
StandardSpatialAttribute<dim> &GSpatialAttrArray<dim>::GetAt(size_t index) const
{
  return (StandardSpatialAttribute<dim>&)m_array.GetAt(index);
}

template<int dim>
StandardSpatialAttribute<dim> &GSpatialAttrArray<dim>::operator[](
  size_t index) const
{
  return (StandardSpatialAttribute<dim>&)m_array.GetAt(index);
}

template<int dim>
bool GSpatialAttrArray<dim>::IsDefined(size_t row) const
{
  return m_array.IsDefined(row);
}

template<int dim>
int GSpatialAttrArray<dim>::Compare(size_t rowA, const AttrArray &arrayB,
                                    size_t rowB) const
{
  return m_array.Compare(rowA, ((GSpatialAttrArray<dim>&)arrayB).m_array, rowB);
}

template<int dim>
int GSpatialAttrArray<dim>::Compare(size_t row, Attribute &value) const
{
  return m_array.Compare(row, value);
}

template<int dim>
size_t GSpatialAttrArray<dim>::GetHash(size_t row) const
{
  return m_array.GetHash(row);
}

template<int dim>
Attribute *GSpatialAttrArray<dim>::GetAttribute(size_t row, bool clone) const
{
  return m_array.GetAttribute(row, clone);
}

template<int dim>
Rectangle<dim> GSpatialAttrArray<dim>::GetBoundingBox(size_t row,
                                                      const Geoid* geoid) const
{
  return GetAt(row).BoundingBox(geoid);
}

template<int dim>
double GSpatialAttrArray<dim>::GetDistance(size_t row,
                                           const Rectangle<dim>& rect,
                                           const Geoid* geoid) const
{
  return GetAt(row).Distance(rect, geoid);
}

template<int dim>
bool GSpatialAttrArray<dim>::Intersects(size_t row, const Rectangle<dim>& rect,
                                        const Geoid* geoid) const
{
  return GetAt(row).Intersects(rect, geoid);
}

template<int dim>
bool GSpatialAttrArray<dim>::IsEmpty(size_t row) const
{
  return GetAt(row).IsEmpty();
}

template<int dim>
GSpatialAttrArrayIterator<dim> GSpatialAttrArray<dim>::GetIterator() const
{
  return GSpatialAttrArrayIterator<dim>(*this);
}

template<int dim>
GSpatialAttrArrayIterator<dim> GSpatialAttrArray<dim>::begin() const
{
  return GSpatialAttrArrayIterator<dim>(*this);
}

template<int dim>
GSpatialAttrArrayIterator<dim> GSpatialAttrArray<dim>::end() const
{
  return GSpatialAttrArrayIterator<dim>();
}

template class GSpatialAttrArray<1>;
template class GSpatialAttrArray<2>;
template class GSpatialAttrArray<3>;
template class GSpatialAttrArray<4>;
template class GSpatialAttrArray<8>;