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
#include "Geoid.h"
#include "RectangleAlgebra.h"

template<int dim>
class SpatialArrayAttribute;
template<int dim>
class SpatialAttrArrayIterator;

template<int dim>
class SpatialAttrArray : public AttrArray
{
public:
  SpatialArrayAttribute<dim> GetAt(size_t row) const
  {
    return SpatialArrayAttribute<dim>(this, row);
  }

  SpatialArrayAttribute<dim> operator[](size_t row) const
  {
    return SpatialArrayAttribute<dim>(this, row);
  }

  virtual const Rectangle<dim> GetBoundingBox(size_t row,
                                              const Geoid* geoid = 0) const = 0;

  virtual double GetDistance(size_t row, const Rectangle<dim>& rect,
                             const Geoid* geoid = 0) const = 0;

  virtual bool Intersects(size_t row, const Rectangle<dim>& rect,
                          const Geoid* geoid = 0) const = 0;

  virtual bool IsEmpty(size_t row) const = 0;

  static unsigned GetDim()
  {
    return dim;
  }

  SpatialAttrArrayIterator<dim> GetIterator() const
  {
    return SpatialAttrArrayIterator<dim>(this);
  }
};

template<int dim>
class SpatialArrayAttribute : public ArrayAttribute
{
public:
  static int GetDim()
  {
    return dim;
  }

  SpatialArrayAttribute()
  {
  }

  SpatialArrayAttribute(const SpatialAttrArray<dim> *block, size_t row) :
    ArrayAttribute(block, row)
  {
  }

  SpatialArrayAttribute(const ArrayAttribute &value) :
    ArrayAttribute(value)
  {
  }

  const SpatialAttrArray<dim> *GetBlock() const
  {
    return (const SpatialAttrArray<dim>*)m_block;
  }

  const Rectangle<dim> GetBoundingBox(const Geoid* geoid = 0) const
  {
    return ((const SpatialAttrArray<dim>*)m_block)->GetBoundingBox(m_row,
                                                                   geoid);
  }

  double GetDistance(const Rectangle<dim>& rect, const Geoid* geoid = 0) const
  {
    return ((const SpatialAttrArray<dim>*)m_block)->GetDistance(m_row, rect,
                                                                geoid);
  }

  bool Intersects(const Rectangle<dim>& rect, const Geoid* geoid = 0) const
  {
    return ((const SpatialAttrArray<dim>*)m_block)->Intersects(m_row, rect,
                                                               geoid);
  }

  bool IsEmpty() const
  {
    return ((const SpatialAttrArray<dim>*)m_block)->IsEmpty(m_row);
  }

  SpatialArrayAttribute<dim> &operator = (const ArrayAttribute &value)
  {
    *((ArrayAttribute*)this) = value;

    return *this;
  }
};

template<int dim>
class SpatialAttrArrayIterator
{
public:
  SpatialAttrArrayIterator() :
    m_count(0),
    m_current(NULL, 0)
  {
  }

  SpatialAttrArrayIterator(const SpatialAttrArray<dim> *instance) :
    m_instance(instance),
    m_count(instance != NULL ? instance->GetCount() : 0),
    m_current(instance, 0)
  {
  }

  bool IsValid() const
  {
    return m_current.m_row < m_count;
  }

  bool MoveToNext()
  {
    if (m_current.m_row < m_count)
    {
      return ++m_current.m_row < m_count;
    }

    return false;
  }

  SpatialArrayAttribute<dim> &GetAttribute()
  {
    return m_current;
  }

private:
  const SpatialAttrArray<dim> *m_instance;

  size_t m_count;

  SpatialArrayAttribute<dim> m_current;
};