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

#include "MObjects.h"

namespace ColumnMovingAlgebra
{

  template<class Unit>
  class MFsObjects : public MObjects
  {
  public:
    MFsObjects();
    MFsObjects(CRelAlgebra::Reader& source);
    MFsObjects(CRelAlgebra::Reader& source, size_t rowsCount);
    MFsObjects(const MFsObjects &array, 
      const CRelAlgebra::SharedArray<const size_t> &filter);
    virtual ~MFsObjects() { }

    virtual AttrArray* Filter(
      CRelAlgebra::SharedArray<const size_t> filter) const;

    virtual Attribute *GetAttribute(size_t row, bool clone = true) const;
    virtual size_t GetCount() const;
    virtual size_t GetSize() const;

    virtual void Save(CRelAlgebra::Writer &target, 
      bool includeHeader = true) const;

    virtual void Append(const CRelAlgebra::AttrArray & array, size_t row);
    virtual void Append(Attribute & value);
    virtual void Remove();
    virtual void Clear();

    virtual bool IsDefined(size_t row) const;

    virtual int  Compare(size_t rowA, const AttrArray& arrayB, 
      size_t rowB) const;
    virtual int  Compare(size_t row, Attribute &value) const;

    virtual size_t GetHash(size_t row) const;

    void atInstant(Instant instant, typename Unit::Instants & result);
    void atPeriods(temporalalgebra::Periods periods, MFsObjects & result);
    void passes(typename Unit::Attr & value, CRelAlgebra::LongInts & result);
    void passes(typename Unit::RAttr & value, CRelAlgebra::LongInts & result);
    void at(typename Unit::Attr & value, MFsObjects & result);
    void at(typename Unit::RAttr & value, MFsObjects & result);

    void addMObject();
    void addUnit(Unit & unit);

  protected:
    struct MObject { 
      int firstUnit; 
      typename Unit::Value minimum, maximum;
    };

    std::shared_ptr<Array<Unit>> m_Units;
    std::shared_ptr<Array<MObject>> m_MObjects;

    int unitFirst(int mobject) const;
    int unitAfterLast(int mobject) const;
    int unitCount(int mobject) const;
    Unit & unit(int index) const;
  };

  template<class Unit>
  MFsObjects<Unit>::MFsObjects() :
    m_Units(std::make_shared<Array<Unit>>()),
    m_MObjects(std::make_shared<Array<MObject>>())
  {
  }

  template<class Unit>
  MFsObjects<Unit>::
  MFsObjects(CRelAlgebra::Reader& source)
  {
    m_Units = std::make_shared<Array<Unit>>(source);
    m_MObjects = std::make_shared<Array<MObject>>(source);
    m_DefTimes = std::make_shared<DefTimes>(source);
  }

  template<class Unit>
  MFsObjects<Unit>::MFsObjects(CRelAlgebra::Reader& source, size_t rowsCount)
  {
    m_Units = std::make_shared<Array<Unit>>(source);
    m_MObjects = std::make_shared<Array<MObject>>(source);
    m_DefTimes = std::make_shared<DefTimes>(source);
  }

  template<class Unit>
  MFsObjects<Unit>::MFsObjects(const MFsObjects &array,
    const CRelAlgebra::SharedArray<const size_t> &filter) :
    MObjects(array, filter),
    m_Units(array.m_Units),
    m_MObjects(array.m_MObjects)
  {
  }
  
  template<class Unit>
  CRelAlgebra::AttrArray* MFsObjects<Unit>::Filter(
    CRelAlgebra::SharedArray<const size_t> filter) const
  {
    return new MFsObjects<Unit>
      (*this, filter);
  }

  template<class Unit>
  Attribute * MFsObjects<Unit>::GetAttribute(size_t row, bool clone) const
  {
    typename Unit::MAttr * mattr = new typename Unit::MAttr(unitCount(row));
    mattr->StartBulkLoad();

    for (int ui = unitFirst(row); ui < unitAfterLast(row); ui++)
      unit(ui).appendTo(*mattr);

    mattr->EndBulkLoad();
    return mattr;
  }

  template<class Unit>
  size_t MFsObjects<Unit>::GetCount() const
  {
    return m_MObjects->size();
  }

  template<class Unit>
  size_t MFsObjects<Unit>::
  GetSize() const
  {
    return  m_Units->savedSize() +
            m_MObjects->savedSize() +
            m_DefTimes->savedSize();
  }

  template<class Unit>
  void MFsObjects<Unit>::Save(CRelAlgebra::Writer &target, 
    bool includeHeader) const
  {
    m_Units->save(target);
    m_MObjects->save(target);
    m_DefTimes->save(target);
  }

  template<class Unit>
  void MFsObjects<Unit>::Append(const CRelAlgebra::AttrArray & array, 
    size_t row)
  {
    const MFsObjects<Unit> & m = 
      static_cast<const MFsObjects<Unit> &>
      (array);

    addMObject();

    for (int ui = m.unitFirst(row); ui < m.unitAfterLast(row); ui++) 
      addUnit(m.unit(ui));
  }

  template<class Unit>
  void MFsObjects<Unit>::Append(Attribute & value)
  {
    auto & m = static_cast<typename Unit::MAttr&>(value);

    addMObject();

    if (!m.IsDefined())
      return;

    for (int ui = 0; ui < m.GetNoComponents(); ui++) {
      Unit u(m, ui);
      addUnit(u);
    }
  }

  template<class Unit>
  void MFsObjects<Unit>::Remove()
  {
    MObject & m = m_MObjects->back();
    m_Units->resize(m.firstUnit);
    m_MObjects->pop_back();

    m_DefTimes->removeRow();
  }

  template<class Unit>
  void MFsObjects<Unit>::Clear()
  {
    m_Units->clear();
    m_MObjects->clear();

    m_DefTimes->clear();
  }

  template<class Unit>
  bool MFsObjects<Unit>::IsDefined(size_t row) const
  {
    return unitCount(row) > 0;
  }

  template<class Unit>
  int MFsObjects<Unit>::Compare(size_t rowA, 
    const CRelAlgebra::AttrArray &arrayB, size_t rowB) 
  const
  {
    const MFsObjects<Unit> & mb = 
      static_cast<const MFsObjects<Unit>&>
      (arrayB);

    int iDiff = unitCount(rowA) - mb.unitCount(rowB);
    if (iDiff != 0)
      return iDiff < 0 ? -1 : 1;

    for (int uiA = unitFirst(rowA), uiB = mb.unitFirst(rowB); 
         uiA < unitAfterLast(rowA); uiA++, uiB++) 
    {
      Unit & a = unit(uiA);
      Unit & b = mb.unit(uiB);

      iDiff = a.interval().compare(b.interval());
      if (iDiff != 0)
        return iDiff < 0 ? -1 : 1;

      iDiff = a.compareValue(b);
      if (iDiff != 0.0)
        return iDiff < 0.0 ? -1 : 1;
    }

    return 0;
  }

  template<class Unit>
  int MFsObjects<Unit>::Compare(size_t row, Attribute &value) const
  {
    MFsObjects<Unit> mrs;
    mrs.Append(value);
    return Compare(row, mrs, 0);
  }

  template<class Unit>
  size_t MFsObjects<Unit>::
  GetHash(size_t row) const
  {
    if (m_Units->size() == 0)
      return 0;

    return (size_t)(unit(0).interval().s ^ unit(0).interval().e);
  }

  template<class Unit>
  void MFsObjects<Unit>::atInstant(Instant instant, 
    typename Unit::Instants & result)
  {
    if (!instant.IsDefined()) {
      for (size_t i = 0; i < GetFilter().GetCount(); i++)
        result.Append(temporalalgebra::Intime<typename Unit::Attr>(instant, 
                                                    Unit::undefinedAttr()));

      return;
    }

    int64_t time = instant.millisecondsToNull();

    for (auto & iterator : GetFilter()) {
      int i = iterator.GetRow();

      Unit * first = m_Units->data() + unitFirst(i);
      Unit * afterLast = m_Units->data() + unitAfterLast(i);
      Unit * u = lower_bound(first, afterLast, time,
        [] (const Unit & a, const int64_t & b) -> bool { 
          return a.interval().e < b || 
                 (a.interval().e == b && !a.interval().rc); 
        });

      if ( u == afterLast || u->interval().s > time || 
          (u->interval().s == time && !u->interval().lc)) {
        result.Append(temporalalgebra::Intime<typename Unit::Attr>(instant, 
                                                    Unit::undefinedAttr()));
        continue;
      }

      result.Append(u->atInstant(instant));
    }
  }

  template<class Unit>
  void MFsObjects<Unit>::
  atPeriods(temporalalgebra::Periods periods, MFsObjects & result)
  {
    if (!periods.IsDefined() || periods.GetNoComponents() == 0) {
      for (size_t i = 0; i < GetFilter().GetCount(); i++)
        result.addMObject();

      return;
    }
    
    for (auto & iterator : GetFilter()) {
      int row = iterator.GetRow();
      result.addMObject();

      int ai = 0, bi = unitFirst(row);

      while (ai < periods.GetNoComponents() && bi < unitAfterLast(row)) {
        temporalalgebra::Interval<Instant> period;
        periods.Get(ai, period);
        Interval a = period;
        Interval b = unit(bi).interval();

        if (a.intersects(b)) {
          Interval c = a.intersection(b);
          Unit u = unit(bi).restrictToInterval(c);
          result.addUnit(u);
        }
        
        if (a.e < b.e || (a.e == b.e && b.rc))
          ai++;
        else
          bi++;
      }
    }
  }

  template<class Unit>
  void MFsObjects<Unit>::passes(typename Unit::Attr & value, 
    CRelAlgebra::LongInts & result)
  {
    result.Clear();

    if (!value.IsDefined())
      return;
    
    for (auto & iterator : GetFilter()) {
      int row = iterator.GetRow();
      MObject& m = (*m_MObjects)[row];

      if (Unit::compare(m.minimum, value) !=  1 && 
          Unit::compare(m.maximum, value) != -1)
      {
        for (int ui = unitFirst(row); ui < unitAfterLast(row); ui++)
          if (unit(ui).passes(value)) {
            result.Append(row);
            break;
          }
      }
    }
  }

  template<class Unit>
  void MFsObjects<Unit>::passes(typename Unit::RAttr & value, 
    CRelAlgebra::LongInts & result)
  {
    result.Clear();

    if (!value.IsDefined())
      return;
      
    typename Unit::Attr max, min;
    value.Maximum(max);
    value.Minimum(min);

    for (auto & iterator : GetFilter()) {
      int row = iterator.GetRow();
      MObject& m = (*m_MObjects)[row];

      if (Unit::compare(m.minimum, max) !=  1 && 
          Unit::compare(m.maximum, min) != -1)
      {
        for (int ui = unitFirst(row); ui < unitAfterLast(row); ui++)
          if (unit(ui).passes(value)) {
            result.Append(row);
            break;
          }
      }
    }
  }

  template<class Unit>
  void MFsObjects<Unit>::at(typename Unit::Attr & value, MFsObjects & result)
  {
    if (!value.IsDefined()) {
      for (size_t i = 0; i < GetFilter().GetCount(); i++)
        result.addMObject();

      return;
    }
    
    for (auto & iterator : GetFilter()) {
      int row = iterator.GetRow();
      MObject& m = (*m_MObjects)[row];

      result.addMObject();

      if (Unit::compare(m.minimum, value) !=  1 && 
          Unit::compare(m.maximum, value) != -1)
      {
        for (int ui = unitFirst(row); ui < unitAfterLast(row); ui++)
          unit(ui).at(value, result);
      }
    }
  }

  template<class Unit>
  void MFsObjects<Unit>::at(typename Unit::RAttr & value, MFsObjects & result)
  {
    if (!value.IsDefined()) {
      for (size_t i = 0; i < GetFilter().GetCount(); i++)
        result.addMObject();

      return;
    }
    
    typename Unit::Attr max, min;
    value.Maximum(max);
    value.Minimum(min);

    for (auto & iterator : GetFilter()) {
      int row = iterator.GetRow();
      MObject& m = (*m_MObjects)[row];

      result.addMObject();

      if (Unit::compare(m.minimum, max) !=  1 && 
          Unit::compare(m.maximum, min) != -1)
      {
        for (int ui = unitFirst(row); ui < unitAfterLast(row); ui++)
          unit(ui).at(value, result);
      }
    }
  }


  template<class Unit>
  void MFsObjects<Unit>::addMObject()
  {
    m_MObjects->emplace_back(MObject{ static_cast<int>(m_Units->size()) });
    m_DefTimes->addRow();
  }

  template<class Unit>
  void MFsObjects<Unit>::addUnit(Unit & unit)
  {
    m_Units->emplace_back(unit);
    m_DefTimes->addInterval(unit.interval());
    
    if (unitCount(m_MObjects->size() - 1) == 0 ||
        unit.minimum() < m_MObjects->back().minimum)
    {
      m_MObjects->back().minimum = unit.minimum();
    }
    
    if (unitCount(m_MObjects->size() - 1) == 0 ||
        m_MObjects->back().maximum < unit.maximum())
    {
      m_MObjects->back().maximum = unit.maximum();
    }
  }
  
  template<class Unit>
  int MFsObjects<Unit>::unitFirst(int mobject) const
  {
    return (*m_MObjects)[mobject].firstUnit;
  }

  template<class Unit>
  int MFsObjects<Unit>::unitAfterLast(int mobject) const
  {
    return mobject < static_cast<int>(m_MObjects->size()) - 1 ? 
      (*m_MObjects)[mobject + 1].firstUnit : 
      m_Units->size();
  }

  template<class Unit>
  int MFsObjects<Unit>::unitCount(int mobject) const
  {
    return unitAfterLast(mobject) - unitFirst(mobject);
  }

  template<class Unit>
  Unit & MFsObjects<Unit>::unit(int index) const
  {
    return (*m_Units)[index];
  }

}
