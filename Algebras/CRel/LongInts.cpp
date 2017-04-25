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

#include "LongInts.h"

#include <limits>
#include "LongInt.h"

using namespace CRelAlgebra;

using std::numeric_limits;
using std::string;

//LongIntEntry------------------------------------------------------------------

const LongIntEntry LongIntEntry::undefined =
  LongIntEntry(numeric_limits<int64_t>::min());

LongIntEntry::LongIntEntry()
{
}

LongIntEntry::LongIntEntry(int64_t value) :
  value(value)
{
}

bool LongIntEntry::IsDefined() const
{
  return value != undefined.value;
}

int LongIntEntry::Compare(const LongIntEntry &value) const
{
  return this->value < value.value ? -1 : this->value > value.value ? 1 : 0;
}

int LongIntEntry::Compare(Attribute &value) const
{
  LongInt &intValue = (LongInt&)value;

  if (intValue.IsDefined())
  {
    if (this->value == undefined.value)
    {
      return -1;
    }

    const int64_t intValueValue = intValue.GetValue();

    return this->value < intValueValue ?
              -1 : this->value > intValueValue ? 1 : 0;
  }

  return this->value == undefined.value ? 0 : 1;
}

bool LongIntEntry::Equals(const LongIntEntry &value) const
{
  return this->value == value.value;
}

bool LongIntEntry::Equals(Attribute &value) const
{
  LongInt &intValue = (LongInt&)value;

  if (intValue.IsDefined())
  {
    return this->value != undefined.value && this->value == intValue.GetValue();
  }

  return this->value == undefined.value;
}

size_t LongIntEntry::GetHash() const
{
  return value;
}

LongIntEntry::operator int64_t() const
{
  return value;
}

//LongInts----------------------------------------------------------------------

LongInts::LongInts()
{
}

LongInts::LongInts(Reader &source) :
  SimpleFSAttrArray<LongIntEntry>(source)
{
}

LongInts::LongInts(Reader &source, size_t rowCount) :
  SimpleFSAttrArray<LongIntEntry>(source, rowCount)
{
}

void LongInts::Append(Attribute &value)
{
  LongInt &longInt = (LongInt&)value;

  Append(longInt.IsDefined() ? LongIntEntry(longInt.GetValue()) :
                               LongIntEntry::undefined);
}

Attribute *LongInts::GetAttribute(size_t row, bool clone) const
{
  const LongIntEntry &entry = GetAt(row);

  return new LongInt(entry.IsDefined(), entry.value);
}