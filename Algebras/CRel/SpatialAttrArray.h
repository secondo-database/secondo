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

#include "AttrArray.h"
#include <cstdint>
#include "Algebras/Geoid/Geoid.h"
#include "Algebras/Rectangle/RectangleAlgebra.h"

/*
Those classes are used for implementing and using types of kinds
SPATIALATTRARRAY1D, SPATIALATTRARRAY2D, SPATIALATTRARRAY3D, SPATIALATTRARRAY4D
and SPATIALATTRARRAY8 D which can be used as column-types in column-oriented
relations.

To implement such a type:
  *your type must derive from ~SpatialAttrArray<dim>~
  *your types type-constructor must derive from ~AttrArrayTypeConstructor~

*/

namespace CRelAlgebra
{
  template<int dim>
  class SpatialAttrArrayEntry;
  template<int dim>
  class SpatialAttrArrayIterator;

  /*
  Abstract class corresponding to kind SPATIALATTRARRAY in the dimension
  specified by ~dim~.

  It derives from ~AttrArray~ and can therefore be used as a colum type in
  column-oriented relations.

  It adds functions related to spacial attribute types.

  */
  template<int dim>
  class SpatialAttrArray : public AttrArray
  {
  public:
    /*
    Creates a ~SpatialAttrArrayEntry<dim>~ representing the entry in the
    specified ~row~.

    Precondition: ~row~ < ~GetCount()~

    */
    SpatialAttrArrayEntry<dim> GetAt(uint64_t row) const;
    SpatialAttrArrayEntry<dim> operator[](uint64_t row) const;

    /*
    Returns the bounding box of the entry in the specified ~row~.

    Precondition: ~row~ < ~GetCount()~

    */
    virtual Rectangle<dim> GetBoundingBox(uint64_t row,
      const Geoid *geoid = nullptr) const = 0;

    /*
    Returns the distance between the entry in the specified ~row~ and ~rect~.

    Precondition: ~row~ < ~GetCount()~

    */
    virtual double GetDistance(uint64_t row, const Rectangle<dim>& rect,
                               const Geoid *geoid = nullptr) const = 0;

    /*
    Checks if the entry in the specified ~row~ intersects with ~rect~.

    Precondition: ~row~ < ~GetCount()~

    */
    virtual bool Intersects(uint64_t row, const Rectangle<dim>& rect,
                            const Geoid *geoid = nullptr) const = 0;

    /*
    Checks if the entry in the specified ~row~ is empty.

    Precondition: ~row~ < ~GetCount()~

    */
    virtual bool IsEmpty(uint64_t row) const = 0;

    /*
    Returns a ~SpatialAttrArrayIterator<dim>~ over the entries.

    */
    SpatialAttrArrayIterator<dim> GetIterator() const;

    /*
    ~AttrArrayIterator~s used (only!) for range-loop support.

    */
    SpatialAttrArrayIterator<dim> begin() const;
    SpatialAttrArrayIterator<dim> end() const;
  };

  /*
  This class represents the entry of a ~SpatialAttrArray<dim>~ by a pointer to
  the array and a row number.

  If either the pointer doesn't point to a ~SpatialAttrArray<dim>~ instance or
  the row number is out of the array's range of rows, the
  ~SpatialAttrArrayEntry<dim>~ is  considered invalid.

  This class doesn't change a ~SpatialAttrArray<dim>~'s reference count.
  If the pointed to array is deleted this ~SpatialAttrArrayEntry<dim>~ becomes
  invalid.

  Using a invalid ~SpatialAttrArrayEntry<dim>~ is considered undefined
  behaviour.

  This class only wraps ~AttrArray~ functions.
  The functions are defined in this header file to enable inlining.

  */
  template<int dim>
  class SpatialAttrArrayEntry : public AttrArrayEntry
  {
  public:
    /*
    Creates a invalid ~AttrArrayEntry~.

    */
    SpatialAttrArrayEntry()
    {
    }


    /*
    Creates a ~SpatialAttrArrayEntry<dim>~ representing the entry in the passed
    ~array~ and ~row~.

    ~array->GetCount()~ < ~row~ <=> the returned entry is valid

    */
    SpatialAttrArrayEntry(const SpatialAttrArray<dim> *array, uint64_t row) :
      AttrArrayEntry(array, row)
    {
    }

    /*
    Creates a ~SpatialAttrArrayEntry<dim>~ from a ~AttrArrayEntry~.

    value is valid <=> the returned entry is valid

    Precondition: ~value.GetArray()~ is instance of ~SpatialAttrArray<dim>~

    */
    SpatialAttrArrayEntry(const AttrArrayEntry &value) :
      AttrArrayEntry(value)
    {
    }

    /*
    Returns the ~SpatialAttrArray<dim>~ pointer

    */
    const SpatialAttrArray<dim> *GetArray() const
    {
      return (const SpatialAttrArray<dim>*)m_array;
    }

    Rectangle<dim> GetBoundingBox(const Geoid* geoid = nullptr) const
    {
      return ((const SpatialAttrArray<dim>*)m_array)->GetBoundingBox(m_row,
                                                                     geoid);
    }

    double GetDistance(const Rectangle<dim>& rect,
                       const Geoid* geoid = nullptr) const
    {
      return ((const SpatialAttrArray<dim>*)m_array)->GetDistance(m_row, rect,
                                                                  geoid);
    }

    bool Intersects(const Rectangle<dim>& rect,
                    const Geoid* geoid = nullptr) const
    {
      return ((const SpatialAttrArray<dim>*)m_array)->Intersects(m_row, rect,
                                                                geoid);
    }

    bool IsEmpty() const
    {
      return ((const SpatialAttrArray<dim>*)m_array)->IsEmpty(m_row);
    }

  private:
    friend class SpatialAttrArrayIterator<dim>;
  };

  /*
  Those ~SpatialAttrArray<dim>~ functions should be inlined if possible.
  Yet they depend on ~SpatialAttrArrayEntry<dim>~ so they are defined here.

  */

  template<int dim>
  inline SpatialAttrArrayEntry<dim> SpatialAttrArray<dim>::GetAt(
    uint64_t row) const
  {
    return SpatialAttrArrayEntry<dim>(this, row);
  }

  template<int dim>
  inline SpatialAttrArrayEntry<dim> SpatialAttrArray<dim>::operator[](
    uint64_t row) const
  {
    return SpatialAttrArrayEntry<dim>(this, row);
  }

  /*
  A very simple implementation of iterator over a ~SpatialAttrArray<dim>~’s
  entries.

  Changes of the ~SpatialAttrArray<dim>~ invalidate the iterator which is not
  reflected by ~SpatialAttrArrayIterator<dim>.IsValid~. Further usage is
  considered undefined behaviour.

  The functions are defined in this header file to enable inlining.

  */
  template<int dim>
  class SpatialAttrArrayIterator
  {
  public:
    /*
    Creates a invalid iterator.

    */
    SpatialAttrArrayIterator() :
      m_count(0),
      m_current(nullptr, 0)
    {
    }

    /*
    Creates a iterator pointing at the first entry in the passed ~array~.
    If the ~array~ is empty the iterator is invalid.

    */
    SpatialAttrArrayIterator(const SpatialAttrArray<dim> *array) :
      m_array(array),
      m_count(array != nullptr ? array->GetCount() : 0),
      m_current(array, 0)
    {
    }

    /*
    Determines if the iterator's current position is valid.

    */
    bool IsValid() const
    {
      return m_current.m_row < m_count;
    }

    /*
    Moves the iterator to the next position.
    Returns true if that position is still valid.

    Precondition: ~IsValid()~

    */
    bool MoveToNext()
    {
      if (m_current.m_row < m_count)
      {
        return ++m_current.m_row < m_count;
      }

      return false;
    }

    /*
    Returns a ~SpatialAttrArrayEntry<dim>~ representing the entry at the
    iterator's current position.

    Precondition: ~IsValid()~

    */
    SpatialAttrArrayEntry<dim> &Get()
    {
      return m_current;
    }

    SpatialAttrArrayEntry<dim> &operator * ()
    {
      return Get();
    }

    /*
    Moves the iterator to the next position.

    Precondition: ~IsValid()~

    */

    SpatialAttrArrayIterator<dim> &operator ++ ()
    {
      MoveToNext();

      return *this;
    }

    /*
    Compares this iterator and the ~other~ iterator for equality.

    */
    bool operator == (const SpatialAttrArrayIterator<dim> &other) const
    {
      if (IsValid())
      {
        if (other.IsValid())
        {
          return m_array == other.m_array &&
                 m_current.m_row == other.m_current.m_row;
        }

        return false;
      }

      return !other.IsValid();
    }

    /*
    Compares this iterator and the ~other~ iterator for inequality.

    */
    bool operator != (const SpatialAttrArrayIterator<dim> &other) const
    {
      return !(*this == other);
    }

  private:
    const SpatialAttrArray<dim> *m_array;

    uint64_t m_count;

    SpatialAttrArrayEntry<dim> m_current;
  };

  /*
  Those ~SpatialAttrArray<dim>~ functions should be inlined if possible.
  Yet they depend on ~SpatialAttrArrayIterator<dim>~ so they are defined here.

  */

  template<int dim>
  inline SpatialAttrArrayIterator<dim> SpatialAttrArray<dim>::GetIterator()
    const
  {
    return SpatialAttrArrayIterator<dim>(this);
  }

  template<int dim>
  inline SpatialAttrArrayIterator<dim> SpatialAttrArray<dim>::begin() const
  {
    return SpatialAttrArrayIterator<dim>(this);
  }

  template<int dim>
  inline SpatialAttrArrayIterator<dim> SpatialAttrArray<dim>::end() const
  {
    return SpatialAttrArrayIterator<dim>();
  }
}
