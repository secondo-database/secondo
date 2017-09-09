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

#include "stdafx.h"
#include "MPoints.h"

using namespace std;

namespace ColumnMovingAlgebra
{
  CRelAlgebra::AttrArray* MPoints::Filter(
    CRelAlgebra::SharedArray<const size_t> filter) const
  {
    return new MPoints(*this, filter);
  }

  size_t MPoints::GetCount() const
  {
    return m_MPointsData->rowCount();
  }

  size_t MPoints::GetSize() const
  {
    return m_MPointsData->savedSize() +
           m_DefTimes->savedSize() +
           sizeof(m_Mbr) +
           sizeof(bool) +
           (m_GridIndex.get() != 0 ? m_GridIndex->savedSize() : 0);
  }

  void MPoints::Save(CRelAlgebra::Writer &target, bool includeHeader) const
  {
    m_MPointsData->save(target);
    m_DefTimes->save(target);
    
    GridIndex::Mbr m = m_Mbr;
    target.WriteOrThrow(reinterpret_cast<char*>(&m), sizeof(m));

    bool hasGrid = m_GridIndex.get() != 0;
    target.WriteOrThrow(reinterpret_cast<char*>(&hasGrid), sizeof(bool));
    if (hasGrid)
      m_GridIndex->save(target);
  }

  void MPoints::Append(const CRelAlgebra::AttrArray & array, size_t row)
  {
    MPoints mpoints = static_cast<const MPoints &>(array);

    addRow();

    MPointsData::UnitIterator i = mpoints.m_MPointsData->unitIterator(row);
    while (i.hasNext()) {
      MPointsData::Unit u = i.next();
      addUnit(u.interval, u.x0, u.y0, u.x1, u.y1);
    }
  }

  void MPoints::Append(Attribute & value)
  {
    temporalalgebra::MPoint & a = static_cast<temporalalgebra::MPoint&>(value);

    addRow();

    if (a.IsDefined()) {
      for (int i = 0; i < a.GetNoComponents(); i++) {
        temporalalgebra::UPoint c;
        a.Get(i, c);
        Interval interval(c.timeInterval);
        addUnit(interval, c.p0.GetX(), c.p0.GetY(), c.p1.GetX(), c.p1.GetY());
      }
    }
  }

  void MPoints::Remove()
  {
    m_MPointsData->removeRow();
    m_DefTimes->removeRow();

    if (m_GridIndex.get() != 0)
      m_GridIndex->removeRow();
  }

  void MPoints::Clear()
  {
    m_MPointsData->clear();
    m_DefTimes->clear();

    if (m_GridIndex.get() != 0)
      m_GridIndex->clear();
  }

  bool MPoints::IsDefined(size_t row) const
  {
    return m_MPointsData->unitCount(row) > 0;
  }

  int MPoints::Compare(size_t rowA, const CRelAlgebra::AttrArray &arrayB,
    size_t rowB) const
  {
    const MPoints & mpointsB = static_cast<const MPoints&>(arrayB);
    int iDiff;
    double dDiff;

    int unitCountA = m_MPointsData->unitCount(rowA);
    int unitCountB = mpointsB.m_MPointsData->unitCount(rowB);

    iDiff = unitCountA - unitCountB;
    if (iDiff != 0)
      return iDiff < 0 ? -1 : 1;

    MPointsData::UnitIterator iA = m_MPointsData->unitIterator(rowA);
    MPointsData::UnitIterator iB = mpointsB.m_MPointsData->unitIterator(rowB);

    while (iA.hasNext()) {
      checkrv(iB.hasNext(), "comparison logical error", 0);

      MPointsData::Unit a = iA.next();
      MPointsData::Unit b = iB.next();

      Interval & intervalA = a.interval;
      Interval & intervalB = b.interval;
      
      iDiff = intervalA.compare(intervalB);
      if (iDiff != 0)
        return iDiff < 0 ? -1 : 1;

      dDiff = a.x0 - b.x0;
      if (dDiff != 0)
        return dDiff < 0 ? -1 : 1;

      dDiff = a.x1 - b.x1;
      if (dDiff != 0)
        return dDiff < 0 ? -1 : 1;

      dDiff = a.y0 - b.y0;
      if (dDiff != 0)
        return dDiff < 0 ? -1 : 1;

      dDiff = a.y1 - b.y1;
      if (dDiff != 0)
        return dDiff < 0 ? -1 : 1;
    }

    return 0;
  }

  int MPoints::Compare(size_t row, Attribute &value) const
  {
    auto mpointB = static_cast<temporalalgebra::MPoint&>(value);

    if (!mpointB.IsDefined())
      return IsDefined(row) ? 1 : 0;

    int iDiff;
    double dDiff;

    int unitCountA = m_MPointsData->unitCount(row);
    int unitCountB = mpointB.GetNoComponents();

    iDiff = unitCountA - unitCountB;
    if (iDiff != 0)
      return iDiff < 0 ? -1 : 1;

    MPointsData::UnitIterator iA = m_MPointsData->unitIterator(row);
    int iB = 0;

    while (iA.hasNext()) {
      checkrv(iB < unitCountB, "comparison logical error", 0);
      temporalalgebra::UPoint b;

      MPointsData::Unit a = iA.next();
      mpointB.Get(iB, b);

      Interval & intervalA = a.interval;
      Interval intervalB(b.timeInterval);

      iDiff = intervalA.compare(intervalB);
      if (iDiff != 0)
        return iDiff < 0 ? -1 : 1;

      dDiff = a.x0 - b.p0.GetX();
      if (dDiff != 0)
        return dDiff < 0 ? -1 : 1;

      dDiff = a.x1 - b.p1.GetX();
      if (dDiff != 0)
        return dDiff < 0 ? -1 : 1;

      dDiff = a.y0 - b.p0.GetY();
      if (dDiff != 0)
        return dDiff < 0 ? -1 : 1;

      dDiff = a.y1 - b.p1.GetY();
      if (dDiff != 0)
        return dDiff < 0 ? -1 : 1;
    }

    return 0;
  }

  size_t MPoints::GetHash(size_t row) const
  {
    if (m_MPointsData->unitCount(row) == 0)
      return 0;

    MPointsData::UnitIterator i = m_MPointsData->unitIterator(row);
    checkrv(i.hasNext(), "hash logical error", 0);
    MPointsData::Unit u = i.next();
    return (size_t)(u.interval.s ^ u.interval.e);
  }

  Attribute * MPoints::GetAttribute(size_t row, bool clone) const
  {
    temporalalgebra::MPoint * attribute = 
      new temporalalgebra::MPoint(m_MPointsData->unitCount(row));
      
    attribute->StartBulkLoad();

    MPointsData::UnitIterator i = m_MPointsData->unitIterator(row);
    while (i.hasNext()) {
      MPointsData::Unit u = i.next();
      Interval & interval = u.interval;
      attribute->Add(temporalalgebra::UPoint(
        temporalalgebra::Interval<Instant>(interval.s, interval.e, 
                                           interval.lc, interval.rc),
        u.x0, u.y0, u.x1, u.y1));
    }

    attribute->EndBulkLoad();
    return attribute;
  }

}
