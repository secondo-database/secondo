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

#include "Algebras/Constraint/ConstraintAlgebra.h"
#include <functional>
#include "SimpleAttrArray.h"
#include "StandardTypes.h"

namespace CRelAlgebra
{
  class RealEntry
  {
  public:
    typedef CcReal AttributeType;

    static const bool isPrecise = false;

    RealEntry()
    {
    }

    RealEntry(double value) :
      value(value)
    {
    }

    RealEntry(const CcReal &value) :
      value(value.IsDefined() ? value.GetValue() : 0.0 / 0.0)
    {
    }

    bool IsDefined() const
    {
      return value != value;
    }

    int Compare(const RealEntry &value) const
    {
      const double a = this->value,
        b = value.value;

      return a != a ? b != b ? 0 : -1 : a < b ? -1 : a > b ? 1 : 0;
    }

    int Compare(const CcReal &value) const
    {
      const double a = this->value;

      if (value.IsDefined())
      {
        const double b = value.GetValue();

        return a != a ? -1 : a < b ? -1 : a > b ? 1 : 0;
      }

      return a != a ? 0 : 1;
    }

    int CompareAlmost(const RealEntry &value) const
    {
      const double a = this->value,
        b = value.value;

      if (a == a)
      {
        if (b == b)
        {
          return Constraint::AlmostEqual(a, b) ? 0 : a < b ? -1 : 1;
        }

        return 1;
      }

      return b != b ? 0 : -1;
    }

    int CompareAlmost(const CcReal &value) const
    {
      const double a = this->value;

      if (a == a)
      {
        if (value.IsDefined())
        {
          const double b = value.GetValue();

          return Constraint::AlmostEqual(a, b) ? 0 : a < b ? -1 : 1;
        }

        return 1;
      }

      return !value.IsDefined() ? 0 : -1;
    }

    bool Equals(const RealEntry &value) const
    {
      const double a = this->value,
        b = value.value;

      return (a != a && b != b) || (a == b);
    }

    bool Equals(const CcReal &value) const
    {
      const double a = this->value;

      return (!value.IsDefined() && a != a) ||
              (a == value.GetValue());
    }

    bool EqualsAlmost(const RealEntry &value) const
    {
      const double a = this->value,
        b = value.value;

      return (a != a && b != b) || AlmostEqual(a, b);
    }

    bool EqualsAlmost(const CcReal &value) const
    {
      const double a = this->value;

      return (!value.IsDefined() && a != a) ||
              AlmostEqual(a, value.GetValue());
    }

    uint64_t GetHash() const
    {
      static const std::hash<double> hashFunction;

      return hashFunction(value);
    }

    CcReal *GetAttribute(bool clone = true) const
    {
      return new CcReal(value == value, value);
    }

    operator double() const
    {
      return value;
    }

    double value;
  };

  typedef SimpleFSAttrArray<RealEntry> Reals;
  typedef SimpleFSAttrArrayIterator<RealEntry> RealsIterator;
}
