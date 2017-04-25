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

#include "Ints.h"

#include <limits>
#include "ListUtils.h"
#include "StandardTypes.h"

using namespace CRelAlgebra;

using listutils::emptyErrorInfo;
using std::numeric_limits;
using std::string;

//IntEntry----------------------------------------------------------------------

const IntEntry IntEntry::undefined =
  IntEntry(numeric_limits<int32_t>::min());

IntEntry::IntEntry()
{
}

IntEntry::IntEntry(int32_t value) :
  value(value)
{
}

bool IntEntry::IsDefined() const
{
  return value != undefined.value;
}

int IntEntry::Compare(const IntEntry &value) const
{
  return this->value < value.value ? -1 : this->value > value.value ? 1 : 0;
}

int IntEntry::Compare(Attribute &value) const
{
  CcInt &intValue = (CcInt&)value;

  if (intValue.IsDefined())
  {
    if (this->value == undefined.value)
    {
      return -1;
    }

    const int32_t intValueValue = intValue.GetValue();

    return this->value < intValueValue ?
              -1 : this->value > intValueValue ? 1 : 0;
  }

  return this->value == undefined.value ? 0 : 1;
}

bool IntEntry::Equals(const IntEntry &value) const
{
  return this->value == value.value;
}

bool IntEntry::Equals(Attribute &value) const
{
  CcInt &intValue = (CcInt&)value;

  if (intValue.IsDefined())
  {
    return this->value != undefined.value && this->value == intValue.GetValue();
  }

  return this->value == undefined.value;
}

size_t IntEntry::GetHash() const
{
  return value;
}

IntEntry::operator int32_t() const
{
  return value;
}

//Ints--------------------------------------------------------------------------

Ints::Ints()
{
}

Ints::Ints(Reader &source) :
  SimpleFSAttrArray<IntEntry>(source)
{
}

Ints::Ints(Reader &source, size_t rowCount) :
  SimpleFSAttrArray<IntEntry>(source, rowCount)
{
}

void Ints::Append(Attribute &value)
{
  const CcInt &ccInt = (const CcInt&)value;

  SimpleFSAttrArray<IntEntry>::Append(ccInt.IsDefined() ?
                                        IntEntry(ccInt.GetValue()) :
                                        IntEntry::undefined);
}

Attribute *Ints::GetAttribute(size_t row, bool clone) const
{
  const IntEntry &entry = GetAt(row);

  return new CcInt(entry.IsDefined(), entry.value);
}

//IntEntry2---------------------------------------------------------------------

void *IntEntry2::Cast(void *target)
{
  return new (target) CcInt();
}

IntEntry2::IntEntry2() :
  m_value(false)
{
}

IntEntry2::IntEntry2(int32_t value) :
  m_value(true, value)
{
}

IntEntry2::IntEntry2(const CcInt &value) :
  m_value(value)
{
}

bool IntEntry2::IsDefined() const
{
  return m_value.IsDefined();
}

int IntEntry2::Compare(const IntEntry2 &value) const
{
  return m_value.Compare(&value.m_value);
}

int IntEntry2::Compare(const Attribute &value) const
{
  return m_value.Compare(&value);
}

int IntEntry2::Equals(const IntEntry2 &value) const
{
  return m_value.Equal(&value.m_value);
}

bool IntEntry2::Equals(const Attribute &value) const
{
  return m_value.Equal(&value);
}

size_t IntEntry2::GetHash() const
{
  return m_value.HashValue();
}

CcInt &IntEntry2::GetValue() const
{
  return m_value;
}

IntEntry2::operator CcInt &() const
{
  return m_value;
}

//Ints2-------------------------------------------------------------------------

Ints2::Ints2()
{
}

Ints2::Ints2(Reader &source) :
  SimpleFSAttrArray<IntEntry2, IntEntry2::Cast>(source)
{
}

Ints2::Ints2(Reader &source, size_t rowCount) :
  SimpleFSAttrArray<IntEntry2, IntEntry2::Cast>(source, rowCount)
{
}

void Ints2::Append(Attribute &value)
{
  SimpleFSAttrArray<IntEntry2, IntEntry2::Cast>::Append((const CcInt&)value);
}

Attribute *Ints2::GetAttribute(size_t row, bool clone) const
{
  return clone ? GetAt(row).GetValue().Clone() : GetAt(row).GetValue().Copy();
}