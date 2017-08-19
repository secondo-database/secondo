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

#include <memory>
#include "AttrArray.h"
#include "Ints.h"
#include "DefTimes.h"
#include "Bools.h"
#include "IPoints.h"
#include "MPointsData.h"
#include "Grid.h"

namespace ColumnMovingAlgebra
{
  class MPoints : public CRelAlgebra::AttrArray
  {
  public:
    MPoints();
    MPoints(CRelAlgebra::Reader& source);
    MPoints(CRelAlgebra::Reader& source, size_t rowsCount);
    MPoints(const MPoints &array, 
            const CRelAlgebra::SharedArray<const size_t> &filter);
    virtual ~MPoints() { }

    virtual AttrArray* Filter(CRelAlgebra::SharedArray<const size_t> 
                              filter) const;

    virtual size_t GetCount() const;
    virtual size_t GetSize() const;
    virtual Attribute *GetAttribute(size_t row, bool clone = true) const;

    virtual void Save(CRelAlgebra::Writer &target, 
                      bool includeHeader = true) const;

    virtual void Append(const CRelAlgebra::AttrArray & array, size_t row);
    virtual void Append(Attribute & value);
    virtual void Remove();
    virtual void Clear();

    virtual bool IsDefined(size_t row) const;

    virtual int  Compare(size_t rowA, const AttrArray& arrayB, 
                               size_t rowB) const;
    virtual int  CompareAlmost(size_t rowA, const AttrArray &arrayB, 
                               size_t rowB) const;
    virtual bool Equals(size_t rowA, const AttrArray &arrayB, 
                               size_t rowB) const;
    virtual bool EqualsAlmost(size_t rowA, const AttrArray &arrayB, 
                               size_t rowB) const;

    virtual int  Compare(size_t row, Attribute &value) const;
    virtual int  CompareAlmost(size_t row, Attribute &value) const;
    virtual bool Equals(size_t row, Attribute &value) const;
    virtual bool EqualsAlmost(size_t row, Attribute &value) const;

    virtual size_t GetHash(size_t row) const;

    void present(Instant instant, CRelAlgebra::LongInts & result);
    void present(temporalalgebra::Periods periods, 
                 CRelAlgebra::LongInts & result);
    void atInstant(Instant instant, IPoints & result);
    void atPeriods(temporalalgebra::Periods periods, MPoints & result);
    void passes(Point & value, CRelAlgebra::LongInts & result);
    void passes(Region & value, CRelAlgebra::LongInts & result);
    void at(Point & value, MPoints & result);
    void at(Region & value, MPoints & result);
    void addRandomRows(CcInt& size, MPoints & result);
    void index(CcInt& temporalSplits, CcInt& spatialSplits, MPoints & result);
    
    void addRow();
    void addUnit(Interval interval, double x0, double y0, 
                                    double x1, double y1);

  private:
    struct GridEntry {
      int row, unitId;
      bool operator==(const GridEntry & b) const;
      bool operator< (const GridEntry & b) const;
    };

    typedef Grid<3, 31, GridEntry> GridIndex;

    shared_ptr<MPointsData> m_MPointsData;
    shared_ptr<DefTimes> m_DefTimes;
    GridIndex::Mbr m_Mbr;
    shared_ptr<GridIndex> m_GridIndex;
  };




  inline MPoints::MPoints() :
    m_MPointsData(make_shared<MPointsData>()),
    m_DefTimes(make_shared<DefTimes>())
  {
  }

  inline MPoints::MPoints(CRelAlgebra::Reader& source)
  {
    m_MPointsData = make_shared<MPointsData>(source);
    m_DefTimes = make_shared<DefTimes>(source);
    source.ReadOrThrow(reinterpret_cast<char*>(&m_Mbr), sizeof(m_Mbr));

    bool hasGrid;
    source.ReadOrThrow(reinterpret_cast<char*>(&hasGrid), sizeof(bool));
    if (hasGrid)
      m_GridIndex = make_shared<GridIndex>(source);
  }

  inline MPoints::MPoints(CRelAlgebra::Reader& source, size_t rowsCount)
  {
    m_MPointsData = make_shared<MPointsData>(source);
    m_DefTimes = make_shared<DefTimes>(source);
    source.ReadOrThrow(reinterpret_cast<char*>(&m_Mbr), sizeof(m_Mbr));

    bool hasGrid;
    source.ReadOrThrow(reinterpret_cast<char*>(&hasGrid), sizeof(bool));
    if (hasGrid)
      m_GridIndex = make_shared<GridIndex>(source);
  }

  inline MPoints::MPoints(const MPoints &array, 
    const CRelAlgebra::SharedArray<const size_t> &filter) :
    AttrArray(filter),
    m_MPointsData(array.m_MPointsData),
    m_DefTimes(array.m_DefTimes),
    m_GridIndex(array.m_GridIndex)
  {
  }

  inline void MPoints::addRow()
  {
    m_MPointsData->addRow();
    m_DefTimes->addRow();

    if (m_GridIndex.get() != 0)
      m_GridIndex->addRow();
  }

  inline void MPoints::addUnit(Interval interval, double x0, double y0, 
                                                  double x1, double y1)
  {
    int unitId;
    m_MPointsData->addUnit(interval, x0, y0, x1, y1, unitId);

    m_DefTimes->addInterval(interval);

    GridIndex::Mbr mbr;
    mbr.l[0] = min(x0, x1);
    mbr.h[0] = max(x0, x1);
    mbr.l[1] = min(y0, y1);
    mbr.h[1] = max(y0, y1);
    mbr.l[2] = static_cast<double>(interval.s);
    mbr.h[2] = static_cast<double>(interval.e);
    
    GridEntry e;
    e.row = m_MPointsData->rowCount() - 1;
    e.unitId = unitId;

    if (m_GridIndex.get() != 0)
      m_GridIndex->add(mbr, e);

    if (m_MPointsData->rowCount() == 1 && m_MPointsData->unitCount(0) == 0)
      m_Mbr = mbr;
    else
      m_Mbr = m_Mbr.unite(mbr);
  }

  inline bool MPoints::GridEntry::operator==(const GridEntry & b) const
  {
    return row == b.row && unitId == b.unitId;
  }

  inline bool MPoints::GridEntry::operator< (const GridEntry & b) const
  {
    if (row < b.row)
      return true;
    if (row > b.row)
      return false;
    
    return unitId < b.unitId;
  }
}
