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

#include "SimpleAttrArray.h"
#include "TemporalAlgebra.h"
#include "SimpleFSAttrArrayIntimeEntry.h"

namespace ColumnMovingAlgebra
{
  struct IPointEntryValue { 
    double x, y;  
    IPointEntryValue(double x, double y);
  };

  class IPointEntry : 
    public SimpleFSAttrArrayIntimeEntry<IPointEntryValue, 
                                        temporalalgebra::IPoint>
  {
  public:
    IPointEntry() = default;
    IPointEntry(bool defined, int64_t time, double x, double y);
    IPointEntry(const temporalalgebra::IPoint &value);

    int Compare(const IPointEntry &value) const;
    int Compare(const temporalalgebra::IPoint &value) const;
    bool Equals(const IPointEntry &value) const;
    bool Equals(const temporalalgebra::IPoint &value) const;


    size_t GetHash() const;
    temporalalgebra::IPoint *GetAttribute(bool clone = true) const;
  };

  typedef CRelAlgebra::SimpleFSAttrArray<IPointEntry> IPoints;
  typedef CRelAlgebra::SimpleFSAttrArrayIterator<IPointEntry> IPointsIterator;



  inline IPointEntryValue::IPointEntryValue(double x, double y) :
    x(x), 
    y(y) 
  {
  }


  inline IPointEntry::IPointEntry(bool defined, int64_t time, 
                                  double x, double y) :
    SimpleFSAttrArrayIntimeEntry(defined, time, IPointEntryValue(x, y))
  {
  }

  inline IPointEntry::IPointEntry(const temporalalgebra::IPoint &value) :
    SimpleFSAttrArrayIntimeEntry(value.IsDefined(), 
                                 value.instant.millisecondsToNull(), 
                                 IPointEntryValue(value.value.GetX(), 
                                 value.value.GetY()))
  {
  }

  inline int IPointEntry::Compare(const IPointEntry &value) const
  {
    int iDiff = (m_Defined ? 1 : 0) - (value.m_Defined ? 1 : 0);
    if (iDiff != 0)
      return iDiff < 0 ? -1 : 1;

    int64_t tDiff = m_Time - value.m_Time;
    if (tDiff != 0)
      return tDiff < 0 ? -1 : 1;

    double dDiff = m_Value.x - value.m_Value.x;
    if (dDiff != 0)
      return dDiff < 0 ? -1 : 1;

    dDiff = m_Value.y - value.m_Value.y;
    if (dDiff != 0)
      return dDiff < 0 ? -1 : 1;

    return 0;
  }

  inline int IPointEntry::Compare(const temporalalgebra::IPoint &value) const
  {
    IPointEntry v(value);
    return Compare(v);
  }

  inline bool IPointEntry::Equals(const IPointEntry &value) const
  {
    return Compare(value) == 0;
  }

  inline bool IPointEntry::Equals(const temporalalgebra::IPoint &value) const
  {
    return Compare(value) == 0;
  }

  inline size_t IPointEntry::GetHash() const
  {
    if (!m_Defined)
      return 0;

    return static_cast<size_t>(m_Value.x) ^ 
           static_cast<size_t>(m_Value.y) ^ 
           static_cast<size_t>(m_Time);
  }

  inline temporalalgebra::IPoint *IPointEntry::GetAttribute(bool clone) const
  {
    if (m_Defined)
      return new temporalalgebra::IPoint(Instant(m_Time), 
                                         Point(true, m_Value.x, m_Value.y));
    else
      return new temporalalgebra::IPoint(0);
  }

}
