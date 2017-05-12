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

#include "AlgebraTypes.h"
#include "AttrArray.h"
#include "Attribute.h"
#include <cstddef>
#include "Geoid.h"
#include "NestedList.h"
#include "ReadWrite.h"
#include "SecondoSMI.h"
#include "Shared.h"
#include "SpatialAttrArray.h"
#include <string>

namespace CRelAlgebra
{
  class GAttrArrayInfo;
  class GAttrArrayIterator;

  /*
  Smartpointer to a ~GAttrArrayInfo~

  */
  typedef Shared<const GAttrArrayInfo> PGAttrArrayInfo;

  /*
  Holds a ~GAttrArray~'s ~count~ and ~flobFileId~ values.
  Moving those values into this class allows writing and reading them with one
  operation and seperate storage (See ~AttrArray.Save~ and
  ~AttrArrayTypeConstructor.Load~).

  */
  class GAttrArrayHeader
  {
  public:
    size_t count;

    SmiFileId flobFileId;

    /*
    Creates a ~GAttrArrayHeader~ with uninitialized values.

    */
    GAttrArrayHeader();

    /*
    Creates a ~GAttrArrayHeader~ with the provided ~count~ and ~flobFileId~.

    */
    GAttrArrayHeader(size_t count, SmiFileId flobFileId);

    /*
    Creates a ~GAttrArrayHeader~ with the ~count~ and ~flobFileId~ from a
    ~AttrArrayHeader~.

    */
    GAttrArrayHeader(const AttrArrayHeader &header);
  };

  /*
  A generic ~AttrArray~ implementation.
  The attribute type is specified by a ~GAttrArrayInfo~.

  The ~Attribute~s are stored in a byte-array of their root blocks.
  ~Flob~s are either copied to a designated file (persistent ~GAttrArray~s) or
  left untouched (transient ~GAttrArray~s).

  */
  class GAttrArray : public AttrArray
  {
  public:
    /*
    Creates a empty ~GAttrArray~ for the attribute type specified by ~info~.
    If persistence is desired a valid ~SmiFileId~ for ~Flob~ storage must be
    provided.

    */
    GAttrArray(const PGAttrArrayInfo &info, SmiFileId flobFileId = 0);

    /*
    Restores a ~GAttrArray~ with attributes of the type specified by ~info~
    from ~source~.

    Preconditions:
      *~source~ must hold data created by ~GAttrArray.Save~ with
       ~includeHeader~ == true.
      *~source~ must hold data created by a ~GAttrArray~ of equal attribute
       type
    */
    GAttrArray(const PGAttrArrayInfo &info, Reader &source);

    /*
    Restores a ~GAttrArray~ with attributes of the type specified by ~info~ from
    ~source~ and ~header~.

    Preconditions:
      *~source~ must hold data created by ~GAttrArray.Save~ with
       ~includeHeader~ == false.
      *~source~ must hold data created by a ~GAttrArray~ of equal attribute
       type.
      *~header.count~ must equal the saved ~GAttrArray~'s row count.
      *~header.flobFile~ must equal the saved ~GAttrArray~'s flobFile count.
    */
    GAttrArray(const PGAttrArrayInfo &info, Reader &source,
               const GAttrArrayHeader &header);

    GAttrArray(const GAttrArray &array,
               const SharedArray<const size_t> &filter);

    virtual ~GAttrArray();

    virtual AttrArray *Filter(const SharedArray<const size_t> filter) const;

    /*
    Returns a shared pointer to the ~GAttrArrayInfo~ specifying this arrays
    attribute type.

    */
    const PGAttrArrayInfo &GetInfo() const;

    //~AttrArray.GetCount~
    virtual size_t GetCount() const;

    /*
    Returns the ammount of memory occupied ~Attribute~ root blocks.
    This does not include allocated but unused memory.

    */
    //~AttrArray.GetSize~
    virtual size_t GetSize() const;

    //~AttrArray.Save~
    virtual void Save(Writer &target, bool includeHeader = true) const;

    //~AttrArray.DeleteRecords~
    virtual void DeleteRecords();

    //~AttrArray.Append~
    virtual void Append(const AttrArray &array, size_t row);

    //~AttrArray.Append~
    virtual void Append(Attribute &value);

    //~AttrArray.Remove~
    virtual void Remove();

    //~AttrArray.Clear~
    virtual void Clear();

    /*
    Returns the ~Attribute~ stored in the specified ~row~.

    Precondition: ~row~ < ~GetCount()~

    */
    Attribute &GetAt(size_t row) const;
    Attribute &operator[](size_t row) const;

    //AttrArray.IsDefined
    virtual bool IsDefined(size_t row) const;

    //AttrArray.Compare
    virtual int Compare(size_t rowA, const AttrArray &arrayB,
                        size_t rowB) const;

    //AttrArray.Compare
    virtual int Compare(size_t row, Attribute &value) const;

    //AttrArray.Equals
    virtual bool Equals(size_t rowA, const AttrArray &arrayB,
                        size_t rowB) const;

    //AttrArray.Equals
    virtual bool Equals(size_t row, Attribute &value) const;

    //AttrArray.GetHash
    virtual size_t GetHash(size_t row) const;

    //AttrArray.GetAttribute
    virtual Attribute *GetAttribute(size_t row, bool clone) const;

    /*
    Returns a ~GAttrArrayIterator~ over the stored attributes.

    */
    GAttrArrayIterator GetIterator() const;

    /*
    Returns ~GAttrArrayIterator~s used (only!) for range-loop support.

    */
    GAttrArrayIterator begin() const;
    GAttrArrayIterator end() const;

  private:
    friend class GAttrArrayIterator;

    GAttrArrayHeader m_header;

    const PGAttrArrayInfo m_info;

    size_t m_capacity,
      m_size;

    SharedArray<char> m_data;

    char *m_attributes,
      *m_attributesEnd;

    /*
    Deleted copy-constructor to prevent copying pointers.

    */
    GAttrArray(const GAttrArray &array) = delete;
  };

  /*
  Specifies and holds some info related to a ~GAttrArray~'s attribute type
  which can be shared among ~GattrArray~s.

  */
  class GAttrArrayInfo
  {
  public:
    int attributeAlgebraId,
      attributeTypeId;

    size_t attributeSize,
      attributeFLOBCount;

    InObject attributeInFunction;

    OutObject attributeOutFunction;

    ObjectCast attributeCastFunction;

    ListExpr attributeType;

    /*
    Creates a uninitialized ~GAttrArrayInfo~

    */
    GAttrArrayInfo();

    /*
    Creates a ~GAttrArrayInfo~ specifying the provided attribute type.

    Note that ~attributeType~ = ~attributeTypeExpr~ but
    ~attributeInFunction~ and ~attributeOutFunction~ only accept numeric type
    representations.

    */
    GAttrArrayInfo(ListExpr attributeTypeExpr);
  };

  /*
  A iterator over the attributes of a ~GAttrArray~.

  Changes of the ~GAttrArray~ invalidate the iterator which is not reflected by
  ~GAttrArrayIterator.IsValid~. Further usage is considered undefined behaviour.

  The functions are defined in this header file to enable inlining.

  */
  class GAttrArrayIterator
  {
  public:
    /*
    Creates a invalid iterator.

    */
    GAttrArrayIterator() :
      m_instance(nullptr),
      m_attributeSize(0),
      m_current(nullptr),
      m_end(nullptr)
    {
    }

    /*
    Creates a iterator pointing at the first entry in the passed ~array~.
    If the ~array~ is empty the iterator is invalid.

    */
    GAttrArrayIterator(const GAttrArray &array) :
      m_instance(&array),
      m_attributeSize(array.m_info->attributeSize),
      m_current(array.m_attributes),
      m_end(array.m_attributesEnd)
    {
    }

    /*
    Determines if the iterator's current position is valid.

    */
    bool IsValid() const
    {
      return m_current < m_end;
    }

    /*
    Returns a ~Attribute~ at the iterator's current position.

    Precondition: ~IsValid()~

    */
    Attribute &Get()
    {
      return *(Attribute*)m_current;
    }

    Attribute &operator * ()
    {
      return Get();
    }

    /*
    Moves the iterator to the next position.
    Returns true if that position is still valid.

    Precondition: ~IsValid()~

    */
    bool MoveToNext()
    {
      if (IsValid())
      {
        m_current += m_attributeSize;

        return IsValid();
      }

      return false;
    }

    /*
    Moves the iterator to the next position.

    Precondition: ~IsValid()~

    */
    GAttrArrayIterator &operator ++ ()
    {
      MoveToNext();

      return *this;
    }

    /*
    Compares this iterator and the ~other~ iterator for equality.

    */
    bool operator == (const GAttrArrayIterator &other) const
    {
      return !(*this != other);
    }

    /*
    Compares this iterator and the ~other~ iterator for inequality.

    */
    bool operator != (const GAttrArrayIterator &other) const
    {
      if (m_current < m_end)
      {
        if (other.m_current < other.m_end)
        {
          return m_current != other.m_current || m_instance != other.m_instance;
        }

        return true;
      }

      return other.m_current < other.m_end;
    }

  private:
    const GAttrArray *m_instance;

    size_t m_attributeSize;

    char *m_current,
      *m_end;
  };

  template<int dim>
  class GSpatialAttrArrayIterator;

  /*
  A generic ~SpatialAttrArray<dim>~ implementation.
  The attribute type is specified by a ~GAttrArrayInfo~.

  This class wraps a ~GAttrArray~ with a attribute type of kind
  SPATIAL1D, SPATIAL2D, SPATIAL3D, SPATIAL4D or SPATIAL8D.

  The fact that ~GAttrArray~ stores ~Attribute~s root blocks, allows to simply
  cast those into ~StandardSpatialAttribute<dim>~s.

  */
  template<int dim>
  class GSpatialAttrArray : public SpatialAttrArray<dim>
  {
  public:
    /*
    The constructors only call ~GAttrArray~'s constructor.

    The attribute type specified by ~info~ must be of kind
    SPATIAL1D, SPATIAL2D, SPATIAL3D, SPATIAL4D or SPATIAL8D.

    */
    GSpatialAttrArray(const PGAttrArrayInfo &info);

    GSpatialAttrArray(const PGAttrArrayInfo &info, SmiFileId flobFileId);

    GSpatialAttrArray(const PGAttrArrayInfo &info, Reader &source);

    GSpatialAttrArray(const PGAttrArrayInfo &info, Reader &source,
                      const GAttrArrayHeader &header);

    GSpatialAttrArray(const GSpatialAttrArray &array,
                      const SharedArray<const size_t> &filter);

    virtual AttrArray *Filter(const SharedArray<const size_t> filter) const;

    //~AttrArray.GetCount~
    virtual size_t GetCount() const;

    //~AttrArray.GetSize~
    virtual size_t GetSize() const;

    //~AttrArray.Save~
    virtual void Save(Writer &target, bool includeHeader = true) const;

    //~AttrArray.DeleteRecords~
    virtual void DeleteRecords();

    //~AttrArray.Append~
    virtual void Append(const AttrArray &array, size_t row);

    //~AttrArray.Append~
    virtual void Append(Attribute &value);

    //~AttrArray.Remove~
    virtual void Remove();

    //~AttrArray.Clear~
    virtual void Clear();

    /*
    Returns the ~StandardSpatialAttribute<dim>~ stored in the specified ~row~.

    Precondition: ~row~ < ~GetCount()~

    */
    StandardSpatialAttribute<dim> &GetAt(size_t index) const;

    StandardSpatialAttribute<dim> &operator[](size_t index) const;

    //~AttrArray.IsDefined~
    virtual bool IsDefined(size_t row) const;

    //~AttrArray.Compare~
    virtual int Compare(size_t rowA, const AttrArray &arrayB,
                        size_t rowB) const;

    //~AttrArray.Compare~
    virtual int Compare(size_t row, Attribute &value) const;

    //~AttrArray.GetHash~
    virtual size_t GetHash(size_t row) const;

    //~AttrArray.GetAttribute~
    virtual Attribute *GetAttribute(size_t row, bool clone) const;

    //~SpatialAttrArray<dim>.GetBoundingBox~
    virtual Rectangle<dim> GetBoundingBox(size_t row,
                                          const Geoid* geoid = 0) const;

    //~SpatialAttrArray<dim>.GetDistance~
    virtual double GetDistance(size_t row, const Rectangle<dim>& rect,
                               const Geoid* geoid = 0) const;

    //~SpatialAttrArray<dim>.Intersects~
    virtual bool Intersects(size_t row, const Rectangle<dim>& rect,
                            const Geoid* geoid = 0) const;

    //~SpatialAttrArray<dim>.IsEmpty~
    virtual bool IsEmpty(size_t row) const;

    /*
    Returns a ~GSpatialAttrArrayIterator<dim>~ over the stored attributes.

    */
    GSpatialAttrArrayIterator<dim> GetIterator() const;

    /*
    Returns ~GSpatialAttrArrayIterator<dim>~s used (only!) for range-loop
    support.

    */
    GSpatialAttrArrayIterator<dim> begin() const;
    GSpatialAttrArrayIterator<dim> end() const;

  private:
    friend class GSpatialAttrArrayIterator<dim>;

    GAttrArray m_array;
  };

  /*
  A iterator over the attributes of a ~GSpatialAttrArray<dim>~.

  Changes of the ~GSpatialAttrArray<dim>~ invalidate the iterator which is not
  reflected by ~IsValid~. Further usage is considered undefined behaviour.

  The functions are defined in this header file to enable inlining.

  */
  template<int dim>
  class GSpatialAttrArrayIterator : public GAttrArrayIterator
  {
  public:
    /*
    Creates a invalid iterator.

    */
    GSpatialAttrArrayIterator()
    {
    }

    /*
    Creates a iterator pointing at the first entry in the passed ~array~.
    If the ~array~ is empty the iterator is invalid.

    */
    GSpatialAttrArrayIterator(const GSpatialAttrArray<dim> &array) :
      GAttrArrayIterator(array.m_array)
    {
    }

    /*
    Returns a ~StandardSpatialAttribute<dim>~ at the iterator's current
    position.

    Precondition: ~IsValid()~

    */
    StandardSpatialAttribute<dim> &Get()
    {
      return (StandardSpatialAttribute<dim>&)GAttrArrayIterator::Get();
    }

    StandardSpatialAttribute<dim> &operator * ()
    {
      return Get();
    }
  };
}