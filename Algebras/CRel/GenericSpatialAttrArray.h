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

#pragma once

#include <cstddef>
#include "GenericAttrArray.h"
#include "Geoid.h"
#include "ReadWrite.h"
#include "SpatialAttrArray.h"

template<int dim>
class GenericSpatialAttrArray : public SpatialAttrArray<dim>
{
public:
  GenericSpatialAttrArray(GenericAttrArray::PInfo info) :
    m_attrArray(info)
  {
  }

  GenericSpatialAttrArray(GenericAttrArray::PInfo info, SmiFileId flobFileId) :
    m_attrArray(info, flobFileId)
  {
  }

  GenericSpatialAttrArray(GenericAttrArray::PInfo info, Reader &source) :
    m_attrArray(info, source)
  {
  }

  virtual size_t GetCount() const
  {
    return m_attrArray.GetCount();
  }

  virtual size_t GetSize() const
  {
    return m_attrArray.GetSize();
  }

  virtual void Save(Writer &target) const
  {
    m_attrArray.Save(target);
  }

  virtual void DeleteRecords()
  {
    m_attrArray.DeleteRecords();
  }

  virtual void CloseFiles()
  {
    m_attrArray.CloseFiles();
  }

  void Clear()
  {
    m_attrArray.Clear();
  }

  virtual void Append(const AttrArray &block, size_t row)
  {
    m_attrArray.Append(((const GenericSpatialAttrArray<dim>&)block)[row]);
  }

  virtual void Append(ListExpr value)
  {
    m_attrArray.Append(value);
  }

  virtual void Append(Attribute &value)
  {
    m_attrArray.Append(value);
  }

  virtual void Remove()
  {
    m_attrArray.Remove();
  }

  Attribute &GetAt(size_t index) const
  {
    return m_attrArray.GetAt(index);
  }

  Attribute &operator[](size_t index) const
  {
    return GetAt(index);
  }

  virtual int Compare(size_t rowA, const AttrArray &blockB, size_t rowB) const
  {
    return m_attrArray.Compare(
      rowA, ((const GenericSpatialAttrArray<dim>&)blockB)[rowB]);
  }

  virtual int Compare(size_t row, Attribute &value) const
  {
    return m_attrArray.Compare(row, value);
  }

  virtual size_t GetHash(size_t row) const
  {
    return m_attrArray.GetHash(row);
  }

  virtual ListExpr GetListExpr(size_t row) const
  {
    return m_attrArray.GetListExpr(row);
  }

  virtual const Rectangle<dim> GetBoundingBox(size_t row,
                                              const Geoid* geoid = 0) const
  {
    return ((StandardSpatialAttribute<dim>&)GetAt(row)).BoundingBox(geoid);
  }

  virtual double GetDistance(size_t row, const Rectangle<dim>& rect,
                             const Geoid* geoid = 0) const
  {
    return ((StandardSpatialAttribute<dim>&)GetAt(row)).Distance(rect, geoid);
  }

  virtual bool Intersects(size_t row, const Rectangle<dim>& rect,
                          const Geoid* geoid = 0) const
  {
    return ((StandardSpatialAttribute<dim>&)GetAt(row)).Intersects(rect, geoid);
  }

  virtual bool IsEmpty(size_t row) const
  {
    return ((StandardSpatialAttribute<dim>&)GetAt(row)).IsEmpty();
  }

private:
  GenericAttrArray m_attrArray;
};