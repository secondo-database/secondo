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

1 IPoints.h

*/

#pragma once

#include "SimpleAttrArray.h"
#include "TemporalAlgebra.h"

namespace ColumnMovingAlgebra
{
/*
1.1 Declaration of class IPointEntry 

~IPointEntry~ represents an intime point in an attribut array

*/

  class IPointEntry
  {
  public:
    typedef temporalalgebra::IPoint AttributeType;
    static const bool isPrecise = true;

/*
default constructors

*/
    IPointEntry() = default;
/*
constructor for specific time and value

*/
    IPointEntry(bool defined, int64_t time, double x, double y);
/*
constructor for conversion of the corresponding attribute type

*/
    IPointEntry(const temporalalgebra::IPoint &value);

/*
~Compare~ compares with another attribut array entry

*/
    int Compare(const IPointEntry &value) const;
/*
or compares with the corresponding attribute type

*/
    int Compare(const temporalalgebra::IPoint &value) const;
/*
~Equals~ checks for equality with another attribut array entry

*/
    bool Equals(const IPointEntry &value) const;
/*
or checks for equality with with the corresponding attribute type

*/
    bool Equals(const temporalalgebra::IPoint &value) const;

/*
~IsDefined~ returns wether the entry is defined

*/
    bool IsDefined() const;
/*
~GetTime~ returns the time point represented by this entry

*/
    int64_t GetTime() const;
/*
~GetHash~ returns a hash value for the entry

*/
    size_t GetHash() const;
/*
~GetAttribute~ converts the entry to the corresponding attribute type

*/
    temporalalgebra::IPoint *GetAttribute(bool clone = true) const;

  private:
/*
~mDefined~ determines whether the entry is defined
~mValueDefined~ dertermines whether the value of the entry is defined

*/
    bool m_Defined, m_ValueDefined;
/*
~mTime~ represents the time, if ~mDefined~ is true

*/
    int64_t m_Time;
/*
~mX~ and ~mY~ represent the position at the time point ~mTime~
if ~mValueDefined~ is true

*/
    double m_X, m_Y;  
  };

/*
1.2 Declaration of IPoints

*/
  typedef CRelAlgebra::SimpleFSAttrArray<IPointEntry> IPoints;
/*
1.2 Declaration of IPointsIterator

*/
  typedef CRelAlgebra::SimpleFSAttrArrayIterator<IPointEntry> IPointsIterator;
/*
1.3 Implementation of class IPointEntry 

constructor for specific time and value

*/
  inline IPointEntry::IPointEntry(bool defined, int64_t time, 
                                  double x, double y) :
    m_Defined(defined),
    m_ValueDefined(true),
    m_Time(time),
    m_X(x),
    m_Y(y)
  {
  }
/*
constructor for conversion of the corresponding attribute type

*/
  inline IPointEntry::IPointEntry(const temporalalgebra::IPoint &value) :
    m_Defined(value.IsDefined()),
    m_ValueDefined(value.value.IsDefined()),
    m_Time(value.instant.millisecondsToNull()),
    m_X(value.value.GetX()),
    m_Y(value.value.GetY())
  {
  }
/*
~Compare~ compares with another attribut array entry

*/
  inline int IPointEntry::Compare(const IPointEntry &value) const
  {
    if (!m_Defined)
      return !value.m_Defined ? 0 : -1;
    else if (!value.m_Defined)
      return 1;

    int64_t tDiff = m_Time - value.m_Time;
    if (tDiff != 0)
      return tDiff < 0 ? -1 : 1;

    if (!m_ValueDefined)
      return !value.m_ValueDefined ? 0 : -1;
    else if (!value.m_ValueDefined)
      return 1;

    double dDiff = m_X - value.m_X;
    if (dDiff != 0)
      return dDiff < 0 ? -1 : 1;

    dDiff = m_Y - value.m_Y;
    if (dDiff != 0)
      return dDiff < 0 ? -1 : 1;

    return 0;
  }
/*
or compares with the corresponding attribute type

*/
  inline int IPointEntry::Compare(const temporalalgebra::IPoint &value) const
  {
    IPointEntry v(value);
    return Compare(v);
  }
/*
~Equals~ checks for equality with another attribut array entry

*/
  inline bool IPointEntry::Equals(const IPointEntry &value) const
  {
    return Compare(value) == 0;
  }
/*
or checks for equality with with the corresponding attribute type

*/
  inline bool IPointEntry::Equals(const temporalalgebra::IPoint &value) const
  {
    return Compare(value) == 0;
  }
/*
~IsDefined~ returns wether the entry is defined

*/
  inline bool IPointEntry::IsDefined() const
  {
    return m_Defined;
  }
/*
~GetTime~ returns the time point represented by this entry

*/
  inline int64_t IPointEntry::GetTime() const {
    return m_Time;
  }
/*
~GetHash~ returns a hash value for the entry

*/
  inline size_t IPointEntry::GetHash() const
  {
    if (!m_Defined)
      return 0;

    return static_cast<size_t>(m_X) ^ 
           static_cast<size_t>(m_Y) ^ 
           static_cast<size_t>(m_Time);
  }
/*
~GetAttribute~ conversion to the corresponding attribute type

*/
  inline temporalalgebra::IPoint *IPointEntry::GetAttribute(bool clone) const
  {
    if (!m_Defined)
      return new temporalalgebra::IPoint(0);
      
    if (!m_ValueDefined) {
      Point r;
      r.SetDefined(false);
      return new temporalalgebra::IPoint(Instant(m_Time), r);
    }
        
    return new temporalalgebra::IPoint(Instant(m_Time), 
                                       Point(true, m_X, m_Y));
  }

}
