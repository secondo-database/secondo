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

#include "Reals.h"

#include <functional>
#include <limits>
#include "StandardTypes.h"

using namespace CRelAlgebra;

using std::hash;
using std::numeric_limits;
using std::string;

//RealEntry---------------------------------------------------------------------

RealEntry::RealEntry()
{
}

RealEntry::RealEntry(double value) :
  value(value)
{
}

bool RealEntry::IsDefined() const
{
  return !isnan(value);
}

int RealEntry::Compare(const RealEntry &value) const
{
  const double a = this->value,
    b = value.value;

  return isnan(a) ? isnan(b) ? 0 : -1 : a < b ? -1 : a > b ? 1 : 0;
}

int RealEntry::Compare(Attribute &value) const
{
  CcReal &realValue = (CcReal&)value;

  if (realValue.IsDefined())
  {
    const double a = this->value,
      b = realValue.GetValue();

    return isnan(a) ? -1 : a < b ? -1 : a > b ? 1 : 0;
  }

  return isnan(this->value) ? 0 : 1;
}

bool RealEntry::Equals(const RealEntry &value) const
{
  const double a = this->value,
    b = value.value;

  return (isnan(a) && isnan(b)) || (a == b);
}

bool RealEntry::Equals(Attribute &value) const
{
  CcReal &realValue = (CcReal&)value;

  return (!realValue.IsDefined() && isnan(this->value)) ||
         (this->value == realValue.GetValue());
}

size_t RealEntry::GetHash() const
{
  static const hash<double> hashFunction;

  return hashFunction(value);
}

RealEntry::operator double() const
{
  return value;
}

//Reals----------------------------------------------------------------------

Reals::Reals()
{
}

Reals::Reals(Reader &source) :
  SimpleFSAttrArray<RealEntry>(source)
{
}

Reals::Reals(Reader &source, size_t rowCount) :
  SimpleFSAttrArray<RealEntry>(source, rowCount)
{
}

void Reals::Append(Attribute &value)
{
  CcReal &real = (CcReal&)value;

  Append(real.IsDefined() ? RealEntry(real.GetValue()) :
                            RealEntry(0 / 0.0));
}

Attribute *Reals::GetAttribute(size_t row, bool clone) const
{
  const RealEntry &entry = GetAt(row);

  return new CcReal(entry.IsDefined(), entry.value);
}