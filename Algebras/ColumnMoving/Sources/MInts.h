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
#include "DefTimes.h"
#include "Bools.h"
#include "IInts.h"

namespace ColumnMovingAlgebra
{
  class MInts : public CRelAlgebra::AttrArray
  {
  public:
    inline MInts();
    inline MInts(CRelAlgebra::Reader& source);
    inline MInts(CRelAlgebra::Reader& source, size_t rowsCount);
    inline MInts(const MInts &array,
      const CRelAlgebra::SharedArray<const size_t> &filter);
    virtual ~MInts() { }

    virtual AttrArray* Filter(CRelAlgebra::SharedArray<const size_t> filter)
      const;

    virtual size_t GetCount() const;
    virtual size_t GetSize() const;
    virtual Attribute *GetAttribute(size_t row, bool clone = true) const;

    virtual void Save(CRelAlgebra::Writer &target, bool includeHeader = true)
      const;

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

    void present(::Instant instant, Bools & result);
    void present(temporalalgebra::Periods periods, Bools & result);
    void atInstant(Instant instant, IInts & result);
    void atPeriods(temporalalgebra::Periods periods, MInts & result);
    void passes(CcInt & value, Bools & result);
    void at(CcInt & value, MInts & result);
    void at(temporalalgebra::RInt & value, MInts & result);

    void addRow();
    void addUnit(int64_t s, int64_t e, bool lc, bool rc, int value);
    int unitCount(int row) const;

  private:
    struct Unit {
      Interval interval;
      int value;
    };

    struct Row {
      int firstUnitIndex;
    };

    shared_ptr<DefTimes> m_DefTimes;
    shared_ptr<Array<Unit>> m_Units;
    shared_ptr<Array<Row>> m_Rows;

    int firstUnitIndex(int row) const;
    int lastUnitIndex(int row) const;
  };




  MInts::MInts() :
    m_DefTimes         (make_shared<DefTimes   >()),
    m_Units            (make_shared<Array<Unit>>()),
    m_Rows             (make_shared<Array<Row> >())
  {
  }

  MInts::MInts(CRelAlgebra::Reader& source)
  {
    m_DefTimes          = make_shared<DefTimes   >(source);
    m_Units             = make_shared<Array<Unit>>(source);
    m_Rows              = make_shared<Array<Row> >(source);
  }

  MInts::MInts(CRelAlgebra::Reader& source, size_t rowsCount)
  {
    m_DefTimes          = make_shared<DefTimes   >(source);
    m_Units             = make_shared<Array<Unit>>(source);
    m_Rows              = make_shared<Array<Row> >(source, rowsCount);
  }

  MInts::MInts(const MInts &array, 
               const CRelAlgebra::SharedArray<const size_t> &filter) :
    AttrArray(filter),
    m_DefTimes(array.m_DefTimes),
    m_Units(array.m_Units),
    m_Rows(array.m_Rows)
  {
  }

  inline void MInts::addUnit(int64_t s, int64_t e, bool lc, bool rc, int value) 
  {
    Unit unit;
    unit.value = value;

    Interval & interval = unit.interval;
    interval.s  = s;
    interval.e  = e;
    interval.lc = lc;
    interval.rc = rc;

    m_Units->push_back(unit);

    m_DefTimes->addInterval(interval);
  }

  inline void MInts::addRow() {
    m_Rows->emplace_back();
    m_Rows->back().firstUnitIndex = m_Units->size();
    m_DefTimes->addRow();
  }

  inline int MInts::firstUnitIndex(int row) const
  {
    return (*m_Rows)[row].firstUnitIndex;
  }

  inline int MInts::lastUnitIndex(int row) const
  {
    if (row == static_cast<int>(m_Rows->size()) - 1)
      return m_Units->size() - 1;
    else
      return (*m_Rows)[row + 1].firstUnitIndex - 1;
  }

  inline int MInts::unitCount(int row) const
  {
    return lastUnitIndex(row) - (*m_Rows)[row].firstUnitIndex + 1;
  }
}
