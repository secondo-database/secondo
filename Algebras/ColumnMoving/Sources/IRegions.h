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
MERCHANTABILITY or FITNESS FOR PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SECONDO; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
----

1 IRegions.h

*/

#pragma once

#include "Attribute.h"
#include "Algebras/CRel/SimpleAttrArray.h"
#include "Algebras/MovingRegion/MovingRegionAlgebra.h"

namespace ColumnMovingAlgebra
{
/*
1.1 Declaration of class IRegionEntry 

~IRegionEntry~ represents an intime real in an attribut array

*/

  class IRegionEntry
  {
  public:
    typedef temporalalgebra::IRegion AttributeType;

    static const bool isPrecise = true;

/*
~GetSize~ returns the size needed to save ~value~ as an attribut array entry

*/
    static size_t GetSize(const temporalalgebra::IRegion & value);
/*
~Write~ saves ~value~ as an attribut array entry

*/
    static void Write(CRelAlgebra::SimpleVSAttrArrayEntry target, 
                      const temporalalgebra::IRegion & value);

/*
default constructors

*/
    IRegionEntry();
/*
constructor for loading from persistant storage

*/
    IRegionEntry(const CRelAlgebra::SimpleVSAttrArrayEntry & value);

/*
~IsDefined~ returns wether the entry is defined

*/
    bool IsDefined() const;
/*
~Compare~ compares with another attribut array entry

*/
    int Compare(const IRegionEntry & value) const;
/*
or compares with the corresponding attribute type

*/
    int Compare(const temporalalgebra::IRegion & value) const;
/*
~Equals~ checks for equality with another attribut array entry

*/
    bool Equals(const IRegionEntry & value) const;
/*
or checks for equality with with the corresponding attribute type

*/
    bool Equals(const temporalalgebra::IRegion & value) const;
/*
~GetHash~ returns a hash value for the entry

*/
    size_t GetHash() const;
/*
~GetAttribute~ converts the entry to the corresponding attribute type

*/
    temporalalgebra::IRegion * GetAttribute(bool clone = true) const;

  private:
    struct Point { double x, y; };
    struct HalfSegment { 
      bool ldp; Point lp, rp; 
      int faceno, cycleno, edgeno, coverageno, partnerno;
      bool insideAbove;
    };

/*
~mDefined~ determines whether the entry is defined

*/
    bool m_Defined;
/*
~mTime~ represents the time, if ~mDefined~ is true

*/
    int64_t m_Time;
/*
the region is represented by half segments

*/
    int m_HalfSegmentsCount;
    HalfSegment * m_HalfSegments;
  };

/*
1.2 Declaration of IRegions

*/
  typedef CRelAlgebra::SimpleVSAttrArray<IRegionEntry> IRegions;
/*
1.3 Implementation of class IRegionEntry 

~GetSize~ returns the size needed to save ~value~ as an attribut array entry

*/
  inline size_t IRegionEntry::GetSize(const temporalalgebra::IRegion &value)
  {
    if (value.IsDefined())
      return sizeof(int64_t) + value.value.Size() * sizeof(HalfSegment);

    return 0;
  }
/*
~Write~ saves ~value~ as an attribut array entry

*/
  inline void IRegionEntry::Write(CRelAlgebra::SimpleVSAttrArrayEntry target, 
                                  const temporalalgebra::IRegion &value)
  {
    if (!value.IsDefined())
      return;

    int64_t * d0 = reinterpret_cast<int64_t*>(target.data);
    *d0 = value.instant.millisecondsToNull();
    d0++;

    HalfSegment * d1 = reinterpret_cast<HalfSegment*>(d0);

    for (int i = 0; i < value.value.Size(); i++, d1++) {
      ::HalfSegment h;
      value.value.Get(i, h);
      d1->ldp = h.IsLeftDomPoint();
      d1->rp.x = h.GetRightPoint().GetX();
      d1->rp.y = h.GetRightPoint().GetY();
      d1->lp.x = h.GetLeftPoint() .GetX();
      d1->lp.y = h.GetLeftPoint() .GetY();
      d1->faceno = h.attr.faceno;
      d1->cycleno = h.attr.cycleno;
      d1->edgeno = h.attr.edgeno;
      d1->coverageno = h.attr.coverageno;
      d1->partnerno = h.attr.partnerno;
      d1->insideAbove = h.attr.insideAbove;
    }
  }
/*
default constructor

*/
  inline IRegionEntry::IRegionEntry()
  {
  }
/*
constructor for reading from persistant storage

*/
  inline IRegionEntry::IRegionEntry(
    const CRelAlgebra::SimpleVSAttrArrayEntry &value) :
    m_Defined(value.size != 0),
    m_HalfSegmentsCount(value.size > 0 ? 
      (value.size - sizeof(int64_t)) / sizeof(HalfSegment) : 
      0)
  {
    if (m_Defined) {
      int64_t * d0 = reinterpret_cast<int64_t*>(value.data);
      m_Time = *d0;
      d0++;

      HalfSegment * d1 = reinterpret_cast<HalfSegment*>(d0);
      m_HalfSegments = d1;
    }
  }
/*
~IsDefined~ returns wether the entry is defined

*/
  inline bool IRegionEntry::IsDefined() const
  {
    return m_Defined;
  }
/*
~Compare~ compares with another attribut array entry

*/
  inline int IRegionEntry::Compare(const IRegionEntry &value) const
  {
    if (!m_Defined)
      return value.m_Defined ? -1 : 0;

    if (!value.m_Defined)
      return 1;

    int64_t di = m_Time - value.m_Time;
    if (di != 0)
      return di < 0 ? -1 : 1;

    int d = m_HalfSegmentsCount - value.m_HalfSegmentsCount;
    if (d != 0)
      return d < 0 ? -1 : 1;

    for (int i = 0; i < m_HalfSegmentsCount; i++) {
      if (m_HalfSegments[i].ldp != value.m_HalfSegments[i].ldp)
        return m_HalfSegments[i].ldp ? -1 : 1;

      double dd = m_HalfSegments[i].lp.x != value.m_HalfSegments[i].lp.x;
      if (dd != 0)
        return dd < 0 ? -1 : 1;

      dd = m_HalfSegments[i].lp.y != value.m_HalfSegments[i].lp.y;
      if (dd != 0)
        return dd < 0 ? -1 : 1;

      dd = m_HalfSegments[i].rp.x != value.m_HalfSegments[i].rp.x;
      if (dd != 0)
        return dd < 0 ? -1 : 1;

      dd = m_HalfSegments[i].rp.y != value.m_HalfSegments[i].rp.y;
      if (dd != 0)
        return dd < 0 ? -1 : 1;

      d = m_HalfSegments[i].faceno != value.m_HalfSegments[i].faceno;
      if (d != 0)
        return d < 0 ? -1 : 1;

      d = m_HalfSegments[i].cycleno != value.m_HalfSegments[i].cycleno;
      if (d != 0)
        return d < 0 ? -1 : 1;

      d = m_HalfSegments[i].edgeno != value.m_HalfSegments[i].edgeno;
      if (d != 0)
        return d < 0 ? -1 : 1;

      d = m_HalfSegments[i].coverageno != value.m_HalfSegments[i].coverageno;
      if (d != 0)
        return d < 0 ? -1 : 1;

      d = m_HalfSegments[i].partnerno != value.m_HalfSegments[i].partnerno;
      if (d != 0)
        return d < 0 ? -1 : 1;
        
      if (m_HalfSegments[i].insideAbove != value.m_HalfSegments[i].insideAbove)
        return m_HalfSegments[i].insideAbove ? -1 : 1;
    }

    return 0;
  }
/*
or compares with the corresponding attribute type

*/
  inline int IRegionEntry::Compare(const temporalalgebra::IRegion &value) const
  {
    if (!m_Defined)
      return value.IsDefined() ? -1 : 0;

    if (!value.IsDefined())
      return 1;

    int64_t di = m_Time - value.instant.millisecondsToNull();
    if (di != 0)
      return di < 0 ? -1 : 1;

    int d = m_HalfSegmentsCount - value.value.Size();
    if (d != 0)
      return d < 0 ? -1 : 1;

    for (int i = 0; i < m_HalfSegmentsCount; i++) {
      ::HalfSegment h;
      value.value.Get(i, h);

      if (m_HalfSegments[i].ldp != h.IsLeftDomPoint())
        return m_HalfSegments[i].ldp ? -1 : 1;

      double dd = m_HalfSegments[i].lp.x != h.GetLeftPoint().GetX();
      if (dd != 0)
        return dd < 0 ? -1 : 1;

      dd = m_HalfSegments[i].lp.y != h.GetLeftPoint().GetY();
      if (dd != 0)
        return dd < 0 ? -1 : 1;

      dd = m_HalfSegments[i].rp.x != h.GetRightPoint().GetX();
      if (dd != 0)
        return dd < 0 ? -1 : 1;

      dd = m_HalfSegments[i].rp.y != h.GetRightPoint().GetY();
      if (dd != 0)
        return dd < 0 ? -1 : 1;

      d = m_HalfSegments[i].faceno != h.attr.faceno;
      if (d != 0)
        return d < 0 ? -1 : 1;

      d = m_HalfSegments[i].cycleno != h.attr.cycleno;
      if (d != 0)
        return d < 0 ? -1 : 1;

      d = m_HalfSegments[i].edgeno != h.attr.edgeno;
      if (d != 0)
        return d < 0 ? -1 : 1;

      d = m_HalfSegments[i].coverageno != h.attr.coverageno;
      if (d != 0)
        return d < 0 ? -1 : 1;

      d = m_HalfSegments[i].partnerno != h.attr.partnerno;
      if (d != 0)
        return d < 0 ? -1 : 1;
        
      if (m_HalfSegments[i].insideAbove != h.attr.insideAbove)
        return m_HalfSegments[i].insideAbove ? -1 : 1;
    }

    return 0;
  }
/*
~Equals~ checks for equality with another attribut array entry

*/
  inline bool IRegionEntry::Equals(const IRegionEntry &value) const
  {
    return Compare(value) == 0;
  }
/*
or checks for equality with with the corresponding attribute type

*/
  inline bool IRegionEntry::Equals(const temporalalgebra::IRegion &value) const
  {
    return Compare(value) == 0;
  }
/*
~GetHash~ returns a hash value for the entry

*/
  inline size_t IRegionEntry::GetHash() const
  {
    if (!m_Defined)
      return 0;

    return static_cast<size_t>(m_Time ^ m_HalfSegmentsCount);
  }
/*
~GetAttribute~ conversion to the corresponding attribute type

*/
  inline temporalalgebra::IRegion *IRegionEntry::GetAttribute(bool clone) const
  {
    if (!m_Defined) {
      temporalalgebra::IRegion * r = new temporalalgebra::IRegion(false);
      r->SetDefined(false);
      return r;
    }

    Region r(m_HalfSegmentsCount);
    
    if (m_HalfSegmentsCount == 0) {
      r.SetDefined(false);
    } else {
      r.StartBulkLoad();

      for (int i = 0; i < m_HalfSegmentsCount; i++) {
        HalfSegment & h = m_HalfSegments[i];
        ::Point lp(true, h.lp.x, h.lp.y);
        ::Point rp(true, h.rp.x, h.rp.y);
        ::HalfSegment hr(h.ldp, lp, rp);
        hr.attr.faceno = h.faceno;
        hr.attr.cycleno = h.cycleno;
        hr.attr.edgeno = h.edgeno;
        hr.attr.coverageno = h.coverageno;
        hr.attr.partnerno = h.partnerno;
        hr.attr.insideAbove = h.insideAbove;
        r.Put(i, hr);
      }

      r.EndBulkLoad();
    }
    
    return new temporalalgebra::IRegion(m_Time, r);
  }
}
