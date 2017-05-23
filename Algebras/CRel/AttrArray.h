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
#include "Attribute.h"
#include <cstddef>
#include "NestedList.h"
#include "ReadWrite.h"
#include "SecondoSMI.h"
#include "Shared.h"
#include <string>
#include "TypeConstructor.h"

/*
Those classes are used for implementing and using types of kind ATTRARRAY which
can be used as column-types in column-oriented relations.

To implement such a type:
  *your type must derive from ~AttrArray~
  *your types type-constructor must derive from ~AttrArrayTypeConstructor~

*/

namespace CRelAlgebra
{
  class AttrArray;
  class AttrArrayEntry;
  class AttrArrayIterator;
  class FilteredAttrArrayIterator;
  class AttrArrayTypeConstructor;

  /*
  This class adds support to filter a ~AttrArray~'s attributes without copying
  them to a new ~AttrArray~.

  It does so by providing its own row numbers which are mapped to the
  ~AttrArray~'s row numbers.

  */
  class AttrArrayFilter
  {
  public:
    /*
    Creates a ~AttrArrayFilter~ for the passed ~array~ containing all
    attributes.

    Note: The ~AttrArray~'s refcounter is not touched

    */
    AttrArrayFilter(const AttrArray &array) :
      m_array(&array)
    {
    }

    /*
    Creates a ~AttrArrayFilter~ for the passed ~array~ containing all only the
    attributes specified by the passed row numbers in ~filter~.

    Note: The ~AttrArray~'s refcounter is not touched

    */
    AttrArrayFilter(const AttrArray &array,
                    const SharedArray<const size_t> &filter) :
      m_array(&array),
      m_filter(filter)
    {
    }

    /*
    Returns the ~AttrArray~'s row number for this ~AttrArrayFilter~'s row number
    ~row~.

    */
    size_t GetAt(size_t row) const
    {
      return m_filter.IsNull() ? row : m_filter[row];
    }
    size_t operator [] (size_t row) const
    {
      return m_filter.IsNull() ? row : m_filter[row];
    }

    /*
    Returns number of row numbers contained in this ~AttrArrayFilter~.

    */
    size_t GetCount() const;

    /*
    Returns a iterator over the ~AttrArray~'s attributes taking this
    ~AttrArrayFilter~ into account

    */
    FilteredAttrArrayIterator GetIterator() const;

    /*
    Only for range-loop support!

    */
    FilteredAttrArrayIterator begin() const;
    FilteredAttrArrayIterator end() const;

  protected:
    const AttrArray * const m_array;

    const SharedArray<const size_t> m_filter;
  };

  /*
  Abstract class corresponding to kind ATTRARRAY

  All attribute array implementations must derive from this class to provide a
  minimum set of functionality which can be used in a generic manner.

  Note: When accessing a ~AttrArray~'s attributes you probably want to use the
  ~AttrArrayFilter~ provided by ~GetFilter~!

  */
  class AttrArray
  {
  public:
    static AttrArrayTypeConstructor *GetTypeConstructor(ListExpr type,
                                                        bool checkKind = true);

    AttrArray() :
      m_filter(*this),
      m_refCount(1)
    {
    }

    /*
    This constructor must be called in the ~Filter~ functions implementation.

    */
    AttrArray(const SharedArray<const size_t> &filter) :
      m_filter(*this, filter),
      m_refCount(1)
    {
    }

    virtual ~AttrArray()
    {
    }

    /*
    Creates a ~AttrArrayEntry~ representing the entry in the specified ~row~.

    Precondition: ~row~ < ~GetCount()~

    */
    AttrArrayEntry GetAt(size_t row) const;
    AttrArrayEntry operator[](size_t row) const;

    /*
    Returns a new ~AttrArray~ instance holding this ~AttrArray~'s attributes
    and a ~AttrArrayFilter~ containing the row numbers in ~filter~ applied to
    it.

    Notes:
    This function is supposed to filter a ~AttrArray~ WITHOUT copying it's
    attributes.

    Make shure to make use of ~AttrArray(const SharedArray<const size_t>&)~ in
    this function's implementation.

    The row numbers are NOT copied to support sharing them among multiple
    ~AttrArray~s so you shouldn't change ~filter~ after passing it to this
    function.

    */
    virtual AttrArray *Filter(const SharedArray<const size_t> filter) const = 0;

    /*
    Returns the applied ~AttrArrayFilter~.

    */
    const AttrArrayFilter &GetFilter() const
    {
      return m_filter;
    }

    /*
    Appends a entry located in the passed ~array~ and ~row~.

    Preconitions:
      *this array and ~array~ are of same type
      *~row~ < ~array.GetCount()~

    */
    virtual void Append(const AttrArray &array, size_t row) = 0;

    /*
    Appends a entry created from the passed ~Attribute~.

    Preconition: ~value~ must be of this array's attribute type

    */
    virtual void Append(Attribute &value) = 0;

    /*
    Appends a entry represented by the provided ~AttrArrayEntry~.

    Preconition: ~value~ represents the entry of a ~AttrArray~ of this array's
    type

    */
    void Append(const AttrArrayEntry &value);

    /*
    Removes the last entry.

    Precondition: ~GetCount()~ > 0

    */
    virtual void Remove() = 0;

    /*
    Removes all entries.

    */
    virtual void Clear() = 0;

    /*
    Returns the number of entries.

    */
    virtual size_t GetCount() const = 0;

    /*
    Returns the size of this ~AttrArray~ in bytes.

    */
    virtual size_t GetSize() const = 0;

    /*
    Returns true if the entry specified by ~row~ is defined, false otherwise.

    Precondition: ~row~ < ~GetCount()~

    */
    virtual bool IsDefined(size_t row) const = 0;

    /*
    All entry comparisons behave like ~Attribute~ comparisons.

    compare(entryA, entryB) < 0 => entryA < entryB
    compare(entryA, entryB) == 0 => entryA == entryB
    compare(entryA, entryB) > 0 => entryA > entryB

    Additionaly to an entrie's value, it's definition state must also be
    considered when evaluating comparisons.

    undefined == undefined
    undefined < value

    */

    /*
    Compares this array's entry in ~rowA~ with ~arrayB~'s entry in ~rowB~.

    Preconditions:
      *this array and ~arrayB~ are of same type
      *~rowA~ < ~GetCount()~
      *~rowB~ < ~arrayB.GetCount()~

    */
    virtual int Compare(size_t rowA, const AttrArray &arrayB,
                        size_t rowB) const = 0;

    /*
    Compares this array's entry in ~row~ with the passed ~Attribute~'s value.

    Preconditions:
      *~row~ < ~GetCount()~
      *~value~ must be of this array's attribute type

    */
    virtual int Compare(size_t row, Attribute &value) const = 0;

    /*
    Compares this array's entry in ~row~ the entry represented by ~value~.

    Preconditions:
      *~row~ < ~GetCount()~
      *~value~ represents the entry of a ~AttrArray~ of this array's type

    */
    int Compare(size_t row, const AttrArrayEntry &value) const;

    /*
    Compares this array's entry in ~rowA~ with ~arrayB~'s entry in ~rowB~.
    For unprecise datatypes the comparison is performed unprecisely.

    Preconditions:
      *this array and ~arrayB~ are of same type
      *~rowA~ < ~GetCount()~
      *~rowB~ < ~arrayB.GetCount()~

    */
    virtual int CompareAlmost(size_t rowA, const AttrArray &arrayB,
                              size_t rowB) const
    {
      return Compare(rowA, arrayB, rowB);
    }

    /*
    Compares this array's entry in ~row~ with the passed ~Attribute~'s value.
    For unprecise datatypes the comparison is performed unprecisely.

    Preconditions:
      *~row~ < ~GetCount()~
      *~value~ must be of this array's attribute type

    */
    virtual int CompareAlmost(size_t row, Attribute &value) const
    {
      return Compare(row, value);
    }

    /*
    Compares this array's entry in ~row~ the entry represented by ~value~.
    For unprecise datatypes the comparison is performed unprecisely.

    Preconditions:
      *~row~ < ~GetCount()~
      *~value~ represents the entry of a ~AttrArray~ of this array's type

    */
    int CompareAlmost(size_t row, const AttrArrayEntry &value) const;

    /*
    Checks this array's entry in ~rowA~ with ~arrayB~'s entry in ~rowB~ for
    equality.

    Preconditions:
      *this array and ~arrayB~ are of same type
      *~rowA~ < ~GetCount()~
      *~rowB~ < ~arrayB.GetCount()~

    */
    virtual bool Equals(size_t rowA, const AttrArray &arrayB, size_t rowB) const
    {
      return Compare(rowA, arrayB, rowB) == 0;
    }

    /*
    Checks this array's entry in ~row~ with the passed ~Attribute~'s value for
    equality.

    Preconditions:
      *~row~ < ~GetCount()~
      *~value~ must be of this array's attribute type

    */
    virtual bool Equals(size_t row, Attribute &value) const
    {
      return Compare(row, value) == 0;
    }

    /*
    Checks this array's entry in ~row~ the entry represented by ~value~ for
    equality.

    Preconditions:
      *~row~ < ~GetCount()~
      *~value~ represents the entry of a ~AttrArray~ of this array's type

    */
    bool Equals(size_t row, const AttrArrayEntry &value) const;

    /*
    Checks this array's entry in ~rowA~ with ~arrayB~'s entry in ~rowB~ for
    unprecise equality.

    Preconditions:
      *this array and ~arrayB~ are of same type
      *~rowA~ < ~GetCount()~
      *~rowB~ < ~arrayB.GetCount()~

    */
    virtual bool EqualsAlmost(size_t rowA, const AttrArray &arrayB,
                              size_t rowB) const
    {
      return CompareAlmost(rowA, arrayB, rowB) == 0;
    }

    /*
    Checks this array's entry in ~row~ with the passed ~Attribute~'s value for
    unprecise equality.

    Preconditions:
      *~row~ < ~GetCount()~
      *~value~ must be of this array's attribute type

    */
    virtual bool EqualsAlmost(size_t row, Attribute &value) const
    {
      return CompareAlmost(row, value) == 0;
    }

    /*
    Checks this array's entry in ~row~ the entry represented by ~value~ for
    unprecise equality.

    Preconditions:
      *~row~ < ~GetCount()~
      *~value~ represents the entry of a ~AttrArray~ of this array's type

    */
    bool EqualsAlmost(size_t row, const AttrArrayEntry &value) const;

    /*
    Returns a hash value for the entry specified by ~row~

    Precondition: ~row~ < ~GetCount()~

    */
    virtual size_t GetHash(size_t row) const = 0;

    /*
    Returns a ~Attribute~ representation of the entry in the specified ~row~.

    The returned value may be only valid during this array's lifetime.
    Passing ~clone~ == true returns a independent ~Attribute~.

    The returned object must be released calling ~DeleteIfAllowed()~!

    Precondition: ~row~ < ~GetCount()~

    */
    virtual Attribute *GetAttribute(size_t row, bool clone = false) const = 0;

    /*
    Writes this array's data into the passed target.
    ~includeHeader~ determines if ~AttrArrayHeader~ data should be omitted.

    */
    virtual void Save(Writer &target, bool includeHeader = true) const = 0;

    /*
    Deletes persistent data created by this array.

    */
    virtual void DeleteRecords()
    {
    }

    /*
    Returns a ~AttrArrayIterator~ over the entries.

    */
    AttrArrayIterator GetIterator() const;

    FilteredAttrArrayIterator GetFilteredIterator() const;

    /*
    ~AttrArrayIterator~s used (only!) for range-loop support.

    */
    AttrArrayIterator begin() const;
    AttrArrayIterator end() const;

    /*
    Increases the reference counter by one.

    */
    void IncRef() const
    {
      ++m_refCount;
    }

    /*
    Decreases the reference counter by one.
    If the reference counter reaches zero this object is deleted.

    */
    void DecRef() const
    {
      if (--m_refCount == 0)
      {
        delete this;
      }
    }

    /*
    Returns the reference count.

    */
    size_t GetRefCount() const
    {
      return m_refCount;
    }

  private:
    const AttrArrayFilter m_filter;

    mutable size_t m_refCount;
  };

  inline size_t AttrArrayFilter::GetCount() const
  {
    return m_filter.IsNull() ? m_array->GetCount() : m_filter.GetCapacity();
  }

  /*
  This class represents the entry of a ~AttrArray~ by a pointer to the array and
  a row number.

  If either the pointer doesn't point to a ~AttrArray~ instance or the row
  number is out of the array's range of rows, the ~AttrArrayEntry~ is considered
  invalid.

  This class doesn't change a ~AttrArray~'s reference count.
  If the pointed to array is deleted this ~AttrArrayEntry~ becomes invalid.

  Using a invalid ~AttrArrayEntry~ is considered undefined behaviour.

  This class only wraps ~AttrArray~ functions.
  The functions are defined in this header file to enable inlining.

  */
  class AttrArrayEntry
  {
  public:
    /*
    Creates a invalid ~AttrArrayEntry~.

    */
    AttrArrayEntry()
    {
    }

    /*
    Creates a ~AttrArrayEntry~ representing the entry in the passed ~array~ and
    ~row~.

    ~array->GetCount()~ < ~row~ <=> the returned entry is valid

    */
    AttrArrayEntry(const AttrArray *array, size_t row) :
      m_array(array),
      m_row(row)
    {
    }

    /*
    Returns the ~AttrArray~ pointer

    */
    const AttrArray *GetArray() const
    {
      return m_array;
    }

    /*
    Returns the row number

    */
    size_t GetRow() const
    {
      return m_row;
    }

    bool IsDefined() const
    {
      return m_array->IsDefined(m_row);
    }

    int Compare(const AttrArray &array, size_t row) const
    {
      return m_array->Compare(m_row, array, row);
    }

    int Compare(const AttrArrayEntry &value) const
    {
      return m_array->Compare(m_row, *value.m_array, value.m_row);
    }

    int Compare(size_t row, Attribute &value) const
    {
      return m_array->Compare(m_row, value);
    }

    bool operator < (const AttrArrayEntry& value) const
    {
      return m_array->Compare(m_row, *value.m_array, value.m_row) < 0;
    }

    bool operator < (Attribute& value) const
    {
      return m_array->Compare(m_row, value) < 0;
    }

    bool operator <= (const AttrArrayEntry& value) const
    {
      return m_array->Compare(m_row, *value.m_array, value.m_row) <= 0;
    }

    bool operator <= (Attribute& value) const
    {
      return m_array->Compare(m_row, value) <= 0;
    }

    bool operator > (const AttrArrayEntry& value) const
    {
      return m_array->Compare(m_row, *value.m_array, value.m_row) > 0;
    }

    bool operator > (Attribute& value) const
    {
      return m_array->Compare(m_row, value) > 0;
    }

    bool operator >= (const AttrArrayEntry& value) const
    {
      return m_array->Compare(m_row, *value.m_array, value.m_row) >= 0;
    }

    bool operator >= (Attribute& value) const
    {
      return m_array->Compare(m_row, value) >= 0;
    }

    bool Equals(const AttrArray &array, size_t row) const
    {
      return m_array->Equals(m_row, array, row);
    }

    bool Equals(const AttrArrayEntry &value) const
    {
      return m_array->Equals(m_row, *value.m_array, value.m_row);
    }

    bool Equals(Attribute &value) const
    {
      return m_array->Equals(m_row, value);
    }

    bool operator == (const AttrArrayEntry& value) const
    {
      return m_array->Equals(m_row, *value.m_array, value.m_row);
    }

    bool operator == (Attribute& value) const
    {
      return m_array->Equals(m_row, value);
    }

    bool operator != (const AttrArrayEntry& value) const
    {
      return !m_array->Equals(m_row, *value.m_array, value.m_row);
    }

    bool operator != (Attribute& value) const
    {
      return !m_array->Equals(m_row, value);
    }

    size_t GetHash() const
    {
      return m_array->GetHash(m_row);
    }

    Attribute *GetAttribute(bool clone = false) const
    {
      return m_array->GetAttribute(m_row, clone);
    }

  protected:
    const AttrArray *m_array;

    size_t m_row;

    friend class AttrArray;
    friend class AttrArrayIterator;
    friend class FilteredAttrArrayIterator;
  };

  /*
  Those ~AttrArray~ functions should be inlined if possible.
  Yet they depend on ~AttrArrayEntry~ so they are defined here.

  */

  inline AttrArrayEntry AttrArray::GetAt(size_t row) const
  {
    return AttrArrayEntry(this, row);
  }

  inline AttrArrayEntry AttrArray::operator[](size_t row) const
  {
    return AttrArrayEntry(this, row);
  }

  inline void AttrArray::Append(const AttrArrayEntry &value)
  {
    Append(*value.m_array, value.m_row);
  }

  inline int AttrArray::Compare(size_t row, const AttrArrayEntry &value) const
  {
    return Compare(row, *value.m_array, value.m_row);
  }

  inline int AttrArray::CompareAlmost(size_t row,
                                      const AttrArrayEntry &value) const
  {
    return CompareAlmost(row, *value.m_array, value.m_row);
  }

  inline bool AttrArray::Equals(size_t row, const AttrArrayEntry &value) const
  {
    return Equals(row, *value.m_array, value.m_row);
  }

  inline bool AttrArray::EqualsAlmost(size_t row,
                                     const AttrArrayEntry &value) const
  {
    return EqualsAlmost(row, *value.m_array, value.m_row);
  }

  /*
  A very simple implementation of iterator over a ~AttrArray~’s entries.

  Changes of the ~AttrArray~ invalidate the iterator which is not reflected by
  ~AttrArrayIterator.IsValid~. Further usage is considered undefined behaviour.

  The functions are defined in this header file to enable inlining.

  */
  class AttrArrayIterator
  {
  public:
    /*
    Creates a invalid iterator.

    */
    AttrArrayIterator() :
      m_count(0),
      m_entry(nullptr, 0)
    {
    }

    /*
    Creates a iterator pointing at the first entry in the passed ~array~.
    If the ~array~ is empty the iterator is invalid.

    */
    AttrArrayIterator(const AttrArray *array) :
      m_array(array),
      m_count(array != nullptr ? array->GetCount() : 0),
      m_entry(array, 0)
    {
    }

    /*
    Determines if the iterator's current position is valid.

    */
    bool IsValid() const
    {
      return m_entry.m_row < m_count;
    }

    /*
    Moves the iterator to the next position.
    ~MoveToNext~ returns true if that position is still valid.

    Precondition: ~IsValid()~

    */
    bool MoveToNext()
    {
      if (m_entry.m_row < m_count)
      {
        return ++m_entry.m_row < m_count;
      }

      return false;
    }

    AttrArrayIterator &operator ++ ()
    {
      MoveToNext();

      return *this;
    }

    /*
    Returns a ~AttrArrayEntry~ representing the entry at the iterator's current
    position.

    Precondition: ~IsValid()~

    */
    AttrArrayEntry &Get()
    {
      return m_entry;
    }

    AttrArrayEntry &operator * ()
    {
      return Get();
    }

    /*
    Compares this iterator and the ~other~ iterator for equality.

    */
    bool operator == (const AttrArrayIterator &other) const
    {
      return !(*this != other);
    }

    /*
    Compares this iterator and the ~other~ iterator for inequality.

    */
    bool operator != (const AttrArrayIterator &other) const
    {
      if (IsValid())
      {
        if (other.IsValid())
        {
          return m_entry.m_row != other.m_entry.m_row ||
                m_array != other.m_array;
        }

        return true;
      }

      return other.IsValid();
    }

  private:
    const AttrArray *m_array;

    size_t m_count;

    AttrArrayEntry m_entry;
  };

  /*
  Those ~AttrArray~ functions should be inlined if possible.
  Yet they depend on ~AttrArrayIterator~ so they are defined here.

  */

  inline AttrArrayIterator AttrArray::GetIterator() const
  {
    return AttrArrayIterator(this);
  }

  inline AttrArrayIterator AttrArray::begin() const
  {
    return GetIterator();
  }

  inline AttrArrayIterator AttrArray::end() const
  {
    return AttrArrayIterator();
  }

  class FilteredAttrArrayIterator
  {
  public:
    /*
    Creates a invalid iterator.

    */
    FilteredAttrArrayIterator() :
      m_count(0),
      m_row(0),
      m_entry(nullptr, 0)
    {
    }

    /*
    Creates a iterator pointing at the first entry in the passed ~array~.
    If the ~array~ is empty the iterator is invalid.

    */
    FilteredAttrArrayIterator(const AttrArray *array) :
      m_filter(array != nullptr ? &array->GetFilter() : nullptr),
      m_count(m_filter != nullptr ? m_filter->GetCount() : 0),
      m_row(0),
      m_entry(array, m_count > 0 ? m_filter->GetAt(0) : 0)
    {
    }

    /*
    Determines if the iterator's current position is valid.

    */
    bool IsValid() const
    {
      return m_row < m_count;
    }

    /*
    Moves the iterator to the next position.
    ~MoveToNext~ returns true if that position is still valid.

    Precondition: ~IsValid()~

    */
    bool MoveToNext()
    {
      if (m_row < m_count)
      {
        if (++m_row < m_count)
        {
          m_entry.m_row = m_filter->GetAt(m_row);

          return true;
        }
      }

      return false;
    }

    FilteredAttrArrayIterator &operator ++ ()
    {
      MoveToNext();

      return *this;
    }

    /*
    Returns a ~AttrArrayEntry~ representing the entry at the iterator's current
    position.

    Precondition: ~IsValid()~

    */
    AttrArrayEntry &Get()
    {
      return m_entry;
    }

    AttrArrayEntry &operator * ()
    {
      return Get();
    }

    /*
    Compares this iterator and the ~other~ iterator for equality.

    */
    bool operator == (const FilteredAttrArrayIterator &other) const
    {
      return !(*this != other);
    }

    /*
    Compares this iterator and the ~other~ iterator for inequality.

    */
    bool operator != (const FilteredAttrArrayIterator &other) const
    {
      if (IsValid())
      {
        if (other.IsValid())
        {
          return m_row != other.m_row || m_filter != other.m_filter;
        }

        return true;
      }

      return other.IsValid();
    }

  private:
    const AttrArrayFilter *m_filter;

    size_t m_count,
      m_row;

    AttrArrayEntry m_entry;
  };

  inline FilteredAttrArrayIterator AttrArrayFilter::GetIterator() const
  {
    return FilteredAttrArrayIterator(m_array);
  }

  inline FilteredAttrArrayIterator AttrArrayFilter::begin() const
  {
    return FilteredAttrArrayIterator(m_array);
  }

  inline FilteredAttrArrayIterator AttrArrayFilter::end() const
  {
    return FilteredAttrArrayIterator();
  }

  inline FilteredAttrArrayIterator AttrArray::GetFilteredIterator() const
  {
    return FilteredAttrArrayIterator(this);
  }

  /*
  Class used to persist some members of a attribute array seperately to avoid
  data redundancy in ~TBlock~s and ~CRel~s.

  */
  class AttrArrayHeader
  {
  public:
    //number of rows in a ~AttrArray~.
    size_t count;

    //id of the file a ~AttrArray~ should use for ~Flob~ storage.
    SmiFileId flobFileId;

    AttrArrayHeader()
    {
    }

    AttrArrayHeader(size_t count, SmiFileId flobFileId) :
      count(count),
      flobFileId(flobFileId)
    {
    }
  };

  /*
  ~AttrArrayManager~ allows to create and load instances of ~AttrArray~s without
  knowledge of the implementing class.

  Instances of ~AttrArrayManager~ can be obtained from a
  ~AttrArrayTypeConstructor~.

  */
  class AttrArrayManager
  {
  public:
    AttrArrayManager();

    virtual ~AttrArrayManager();

    /*
    Creates a new instance of ~AttrArray~, providing a file id for ~Flob~
    storage.

    ~flobFileId~ == 0 => no ~Flob~ storage

    The returned object must be released using ~DecRef~.

    */
    virtual AttrArray *Create(SmiFileId flobFileId) = 0;

    /*
    Loads a new instance of ~AttrArray~ from the provided ~source~.

    Preconditions:
      *~source~ holds data created by calling a ~AttrArray~'s ~Save~ function
       with ~includeHeader~ == true
      *the ~AttrArray~ was created by this ~AttrArrayManager~ implementation

    The returned object must be released using ~DecRef~.

    */
    virtual AttrArray *Load(Reader &source) = 0;

    /*
    Loads a new instance of ~AttrArray~ from the provided ~source~ passing
    ~AttrArrayHeader~ data seperately.

    Preconditions:
      *~source~ holds data created by calling a ~AttrArray~'s ~Save~ function
       with ~includeHeader~ == false
      *the ~AttrArray~ was created by this ~AttrArrayManager~ implementation

    The returned object must be released using ~DecRef~.

    */
    virtual AttrArray *Load(Reader &source, const AttrArrayHeader &header) = 0;

    /*
    Increases the reference counter by one.

    */
    void IncRef() const;

    /*
    Decreases the reference counter by one.
    If the reference counter reaches zero this object is deleted.

    */
    void DecRef() const;


    /*
    Returns the reference count.

    */
    size_t GetRefCount() const;

  private:
    mutable size_t m_refCount;
  };

  /*
  This function is supposed to return the attribute type for a attribute-array
  type ~typeExpr~.

  If ~numeric~ the provided and returned types must be in their numeric
  representation (see ~SecondoCatalog~).

  */
  typedef ListExpr (*AttrArrayTypeFunction)(ListExpr typeExpr, bool numeric);

  /*
  This function is supposed to return instance of ~AttrArrayManager~ for the
  provided attribute type.

  Normally ~attributeType~ was previously determined by a
  ~AttrArrayTypeFunction~.

  */
  typedef AttrArrayManager *(*AttrArrayManagerFunction)(ListExpr attributeType);

  /*
  ~TypeConstructor~ for ATTRARRAY types.
  Every ~TypeConstructor~ for a ATTRARRAY type must derive from
  ~AttrArrayTypeConstructor~ to provide functions used by ~TBlock~ and ~CRel~.

  */
  class AttrArrayTypeConstructor : public TypeConstructor
  {
  public:
    /*
    This function returns a default ATTRARRAY type with the provided attribute
    type.

    If ~numeric~ the provided and returned types must be in their numeric
    representation (see ~SecondoCatalog~).

    */
    static ListExpr GetDefaultAttrArrayType(ListExpr attributeType,
                                            bool numeric);

    /*
    Creates a ~AttrArrayTypeConstructor~ with the provided ~name~ and functions
    and accociates it with ATTRARRAY.

    */
    AttrArrayTypeConstructor(const std::string& name, TypeProperty typeProperty,
                             TypeCheckFunction typeCheck,
                             AttrArrayTypeFunction attributeType,
                             AttrArrayManagerFunction manager);

    /*
    Creates a ~AttrArrayTypeConstructor~ with the provided ~name~ and functions
    and accociates it with ATTRARRAY.

    */
    AttrArrayTypeConstructor(const std::string& name, TypeProperty typeProperty,
                             OutObject out, InObject in, ObjectCreation create,
                             ObjectDeletion del, ObjectOpen open,
                             ObjectSave save, ObjectClose close,
                             ObjectClone clone, ObjectCast cast,
                             ObjectSizeof sizeOf, TypeCheckFunction typeCheck,
                             AttrArrayTypeFunction attributeType,
                             AttrArrayManagerFunction manager);

    /*
    Returns the attribute type for this ~TypeConstructor~'s ATTRARRAY type
    ~typeExpr~.

    If ~numeric~ the provided and returned types must be in their numeric
    representation (see ~SecondoCatalog~).

    */
    ListExpr GetAttributeType(ListExpr typeExpr, bool numeric);

    /*
    Returns a instance of ~AttrArrayManager~ for this ~TypeConstructor~'s
    ATTRARRAY type with the provided attribute type.

    The returned object must be released using ~DecRef~.

    */
    AttrArrayManager *CreateManager(ListExpr attributeType);

  protected:
    /*
    Default implementations of some ~TypeConstructor~ functions for
    ATTRARRAY types.

    */

    static Word DefaultIn(ListExpr typeExpr, ListExpr value, int errorPos,
                          ListExpr &errorInfo, bool &correct);

    static ListExpr DefaultOut(ListExpr typeExpr, Word value);

    static Word DefaultCreate(const ListExpr typeExpr);

    static void DefaultDelete(const ListExpr typeExpr, Word &value);

    static bool DefaultOpen(SmiRecord &valueRecord, size_t &offset,
                            const ListExpr typeExpr, Word &value);

    static bool DefaultSave(SmiRecord &valueRecord, size_t &offset,
                            const ListExpr typeExpr, Word &value);

    static void DefaultClose(const ListExpr typeExpr, Word &value);

    static void *DefaultCast(void *addr);

    static int DefaultSizeOf();

    static Word DefaultClone(const ListExpr typeExpr, const Word &value);

  private:
    AttrArrayTypeFunction m_getAttributeType;

    AttrArrayManagerFunction m_createManager;
  };
}