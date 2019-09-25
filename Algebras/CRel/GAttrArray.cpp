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


using listutils::emptyErrorInfo;
using std::max;
using std::string;

extern AlgebraManager *am;


namespace CRelAlgebra {

GAttrArrayInfo::GAttrArrayInfo()
{
}

GAttrArrayInfo::GAttrArrayInfo(ListExpr attributeTypeExpr)
{
  int attributeAlgebraId,
    attributeTypeId;

  ResolveTypeOrThrow(attributeTypeExpr, attributeAlgebraId, attributeTypeId);

  TypeConstructor &typeConstructor = *am->GetTC(attributeAlgebraId,
                                                attributeTypeId);

  attributeSize =  typeConstructor.SizeOf();

  attributeCastFunction = am->Cast(attributeAlgebraId, attributeTypeId);

  attributeFLOBCount = max(typeConstructor.NumOfFLOBs(), 0);
}

GAttrArray::GAttrArray(const PGAttrArrayInfo &info, SmiFileId flobFileId) :
  m_header(0, flobFileId),
  m_info(info),
  m_capacity(0),
  m_size(sizeof(GAttrArray)),
  m_attributesEnd(nullptr)
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
  m_size(sizeof(GAttrArray) + (m_header.count * info->attributeSize)),
  m_data(m_header.count * info->attributeSize)
{
  const uint64_t count = m_header.count;

  if (count > 0)
  {
    const uint64_t attributeSize = info->attributeSize;

    char *attribute = m_data.GetPointer();

    m_attributesEnd = attribute + (attributeSize * count);

    source.ReadOrThrow(attribute, attributeSize * count);

    ObjectCast cast = m_info->attributeCastFunction;

    char *end = m_attributesEnd;

    while (attribute < end)
    {
      cast((Attribute*)attribute);

      attribute += attributeSize;
    }
  }
  else
  {
    m_attributesEnd = nullptr;
  }
}

GAttrArray::GAttrArray(const GAttrArray &array,
                       const SharedArray<const uint64_t> &filter) :
  AttrArray(filter),
  m_header(array.m_header),
  m_info(array.m_info),
  m_capacity(array.m_capacity),
  m_size(array.m_size),
  m_data(array.m_data),
  m_attributesEnd(array.m_attributesEnd)
{
}

GAttrArray::~GAttrArray()
{
  const uint64_t count = m_header.count;

  if (count > 0)
  {
    const GAttrArrayInfo &info = *m_info;

    const uint64_t flobCount = info.attributeFLOBCount,
      attributeSize = info.attributeSize;

    if (flobCount > 0)
    {
      char *currentAttribute = m_data.GetPointer(),
           *attributesEnd = m_attributesEnd;

      while (currentAttribute < attributesEnd)
      {
        Attribute *attribute = (Attribute*)currentAttribute;

        for (uint64_t i = 0; i < flobCount; i++)
        {
          attribute->GetFLOB(i)->~Flob();
        }

        currentAttribute += attributeSize;
      }
    }
  }
}


AttrArray *GAttrArray::Filter(const SharedArray<const uint64_t> filter) const
{
  return new GAttrArray(*this, filter);
}

const PGAttrArrayInfo &GAttrArray::GetInfo() const
{
  return m_info;
}

uint64_t GAttrArray::GetCount() const
{
  return m_header.count;
}

uint64_t GAttrArray::GetSize() const
{
  return m_size;
}

void GAttrArray::Save(Writer &target, bool includeHeader) const
{
  if (includeHeader)
  {
    target.WriteOrThrow(m_header);
  }

  const uint64_t count = m_header.count;

  if (count > 0)
  {
    target.WriteOrThrow(m_data.GetPointer(), count * m_info->attributeSize);
  }
}

void GAttrArray::DeleteRecords()
{
  /*
  Delete all ~Flob~ records.

  */

  const GAttrArrayInfo &info = *m_info;

  const uint64_t flobCount = info.attributeFLOBCount;

  if (m_header.flobFileId != 0 && flobCount > 0)
  {
    const uint64_t count = m_header.count;

    if (count > 0)
    {
      const uint64_t attributeSize = info.attributeSize;

      char *currentAttributeData = m_data.GetPointer(),
        *attributeDataEnd = m_attributesEnd;

      while (currentAttributeData < attributeDataEnd)
      {
        Attribute *attribute = (Attribute*)currentAttributeData;

        for (uint64_t i = 0; i < flobCount; i++)
        {
          attribute->GetFLOB(i)->destroy();
        }

        currentAttributeData += attributeSize;
      }
    }
  }
}

void GAttrArray::Append(const AttrArray &array, uint64_t row)
{
  Append(((GAttrArray&)array).GetAt(row));
}

void GAttrArray::Append(Attribute &value)
{
  const GAttrArrayInfo &info = *m_info;
  const uint64_t attributeSize = info.attributeSize;

  uint64_t capacity = m_capacity,
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

      attributeData = new char[attributeSize];
      attributeDataEnd = m_attributesEnd = attributeData;

      m_data = SharedArray<char>(attributeData, attributeSize);
    }
    else
    {
      const uint64_t oldByteCapacity = capacity * attributeSize,
        byteCapacity = oldByteCapacity + oldByteCapacity;

      capacity += capacity;

      attributeData = new char[byteCapacity];
      attributeDataEnd = m_attributesEnd = attributeData + oldByteCapacity;

      //copy existing values
      memcpy(attributeData, m_data.GetPointer(), oldByteCapacity);

      m_data = SharedArray<char>(attributeData, byteCapacity);
    }

    m_capacity = capacity;
  }
  else
  {
    attributeData = m_data.GetPointer();
    attributeDataEnd = m_attributesEnd;
  }

  Attribute *attribute = (Attribute*)attributeDataEnd;

  //copy the new attribute's data
  memcpy((void*)attribute, (void*) &value, attributeSize);

  const uint64_t flobCount = info.attributeFLOBCount;

  //copy Flobs?
  if (flobCount > 0)
  {
    const SmiFileId flobFileId = m_header.flobFileId;

    if (flobFileId != 0)
    {
      for (uint64_t j = 0; j < flobCount; j++)
      {
        Flob &sourceFlob = *value.GetFLOB(j),
          *targetFlob = attribute->GetFLOB(j);

        new (targetFlob) Flob(0);

        sourceFlob.saveToFile(flobFileId, 0, *targetFlob);
      }
    }
    else
    {
      for (uint64_t j = 0; j < flobCount; j++)
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
  m_attributesEnd += attributeSize;
}

void GAttrArray::Remove()
{
  /*
  Remove last ~Attribute~ and delete it's copied ~Flob~s

  */
  const GAttrArrayInfo &info = *m_info;

  const uint64_t flobCount = info.attributeFLOBCount,
   count = m_header.count;

  if (count > 0 && flobCount > 0 && m_header.flobFileId != 0)
  {
    const uint64_t attributeSize = info.attributeSize;

    Attribute *attribute = (Attribute*)(m_attributesEnd -= attributeSize);

    for (uint64_t i = 0; i < flobCount; i++)
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

    m_attributesEnd = m_data.GetPointer();

    m_size = sizeof(GAttrArray);
  }
}

Attribute &GAttrArray::GetAt(uint64_t row) const
{
  return *(Attribute*)(m_data.GetPointer() + (row * m_info->attributeSize));
}

Attribute &GAttrArray::operator[](uint64_t row) const
{
  return *(Attribute*)(m_data.GetPointer() + (row * m_info->attributeSize));
}

bool GAttrArray::IsDefined(uint64_t row) const
{
  return GetAt(row).IsDefined();
}

int GAttrArray::Compare(uint64_t rowA, const AttrArray &arrayB,
                        uint64_t rowB) const
{
  return GetAt(rowA).Compare(&((GAttrArray&)arrayB).GetAt(rowB));
}

int GAttrArray::Compare(uint64_t row, Attribute &value) const
{
  return GetAt(row).Compare(&value);
}

int GAttrArray::CompareAlmost(uint64_t rowA, const AttrArray &arrayB,
                              uint64_t rowB) const
{
  return GetAt(rowA).CompareAlmost(&((GAttrArray&)arrayB).GetAt(rowB));
}

int GAttrArray::CompareAlmost(uint64_t row, Attribute &value) const
{
  return GetAt(row).CompareAlmost(&value);
}

bool GAttrArray::Equals(uint64_t rowA, const AttrArray &arrayB,
                              uint64_t rowB) const
{
  return GetAt(rowA).Equal(&((GAttrArray&)arrayB).GetAt(rowB));
}

bool GAttrArray::Equals(uint64_t row, Attribute &value) const
{
  return GetAt(row).Equal(&value);
}

uint64_t GAttrArray::GetHash(uint64_t row) const
{
  return GetAt(row).HashValue();
}

Attribute *GAttrArray::GetAttribute(uint64_t row, bool clone) const
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

GAttrArrayHeader::GAttrArrayHeader(uint64_t count, SmiFileId flobFileId) :
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
GSpatialAttrArray<dim>::GSpatialAttrArray(const GSpatialAttrArray &array,
  const SharedArray<const uint64_t> &filter) :
  m_array(array.m_array, filter)
{
}

template<int dim>
AttrArray *GSpatialAttrArray<dim>::Filter(
  const SharedArray<const uint64_t> filter) const
{
  return new GSpatialAttrArray<dim>(*this, filter);
}

template<int dim>
uint64_t GSpatialAttrArray<dim>::GetCount() const
{
  return m_array.GetCount();
}

template<int dim>
uint64_t GSpatialAttrArray<dim>::GetSize() const
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
void GSpatialAttrArray<dim>::Append(const AttrArray &array, uint64_t row)
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
StandardSpatialAttribute<dim> &GSpatialAttrArray<dim>::GetAt(
  uint64_t index) const
{
  return (StandardSpatialAttribute<dim>&)m_array.GetAt(index);
}

template<int dim>
StandardSpatialAttribute<dim> &GSpatialAttrArray<dim>::operator[](
  uint64_t index) const
{
  return (StandardSpatialAttribute<dim>&)m_array.GetAt(index);
}

template<int dim>
bool GSpatialAttrArray<dim>::IsDefined(uint64_t row) const
{
  return m_array.IsDefined(row);
}

template<int dim>
int GSpatialAttrArray<dim>::Compare(uint64_t rowA, const AttrArray &arrayB,
                                    uint64_t rowB) const
{
  return m_array.Compare(rowA, ((GSpatialAttrArray<dim>&)arrayB).m_array, rowB);
}

template<int dim>
int GSpatialAttrArray<dim>::Compare(uint64_t row, Attribute &value) const
{
  return m_array.Compare(row, value);
}

template<int dim>
int GSpatialAttrArray<dim>::CompareAlmost(uint64_t rowA,
                                          const AttrArray &arrayB,
                                          uint64_t rowB) const
{
  return m_array.CompareAlmost(rowA,
                               ((GSpatialAttrArray<dim>&)arrayB).m_array, rowB);
}

template<int dim>
int GSpatialAttrArray<dim>::CompareAlmost(uint64_t row, Attribute &value) const
{
  return m_array.CompareAlmost(row, value);
}

template<int dim>
bool GSpatialAttrArray<dim>::Equals(uint64_t rowA, const AttrArray &arrayB,
                                   uint64_t rowB) const
{
  return m_array.Equals(rowA, ((GSpatialAttrArray<dim>&)arrayB).m_array, rowB);
}

template<int dim>
bool GSpatialAttrArray<dim>::Equals(uint64_t row, Attribute &value) const
{
  return m_array.Equals(row, value);
}

template<int dim>
bool GSpatialAttrArray<dim>::EqualsAlmost(uint64_t rowA,
                                          const AttrArray &arrayB,
                                          uint64_t rowB) const
{
  return m_array.EqualsAlmost(rowA,
                              ((GSpatialAttrArray<dim>&)arrayB).m_array, rowB);
}

template<int dim>
bool GSpatialAttrArray<dim>::EqualsAlmost(uint64_t row, Attribute &value) const
{
  return m_array.EqualsAlmost(row, value);
}

template<int dim>
uint64_t GSpatialAttrArray<dim>::GetHash(uint64_t row) const
{
  return m_array.GetHash(row);
}

template<int dim>
Attribute *GSpatialAttrArray<dim>::GetAttribute(uint64_t row, bool clone) const
{
  return m_array.GetAttribute(row, clone);
}

template<int dim>
Rectangle<dim> GSpatialAttrArray<dim>::GetBoundingBox(uint64_t row,
                                                      const Geoid* geoid) const
{
  return GetAt(row).BoundingBox(geoid);
}

template<int dim>
double GSpatialAttrArray<dim>::GetDistance(uint64_t row,
                                           const Rectangle<dim>& rect,
                                           const Geoid* geoid) const
{
  return GetAt(row).Distance(rect, geoid);
}

template<int dim>
bool GSpatialAttrArray<dim>::Intersects(uint64_t row,
                                        const Rectangle<dim>& rect,
                                        const Geoid* geoid) const
{
  return GetAt(row).Intersects(rect, geoid);
}

template<int dim>
bool GSpatialAttrArray<dim>::IsEmpty(uint64_t row) const
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

} // end of namepsace
