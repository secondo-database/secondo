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

1 MFsObjects.h

*/

#pragma once

#include "MObjects.h"

namespace ColumnMovingAlgebra
{

/*
1.1 Declaration of ~MFsObjects~

~MFsObjects~ is a template class for attribute arrays of one-dimensional
moving objects.
It is used to build the attribute arrays for moving booleans, moving
integers moving reals and moving strings. 

*/
  template<class Unit>
  class MFsObjects : public MObjects
  {
  public:
/*
constructors required by the crel algebra for all attribute arrays.

*/
    MFsObjects();
    MFsObjects(CRelAlgebra::Reader& source);
    MFsObjects(CRelAlgebra::Reader& source, uint64_t rowsCount);
    MFsObjects(const MFsObjects &array, 
      const CRelAlgebra::SharedArray<const uint64_t> &filter);
/*
destructor

*/
    virtual ~MFsObjects() { }

/*
1.1.1 CRel Algebra Interface

the following functions are required by the crel algebra for all attribute 
arrays.

~Filter~ returns a duplicate of this attribut array with the speficied filter.

 
*/
    virtual AttrArray* Filter(
      CRelAlgebra::SharedArray<const uint64_t> filter) const;

/*
~GetAttribute~ converts the entry in ~row~ to a attribute for row oriented
relations.

*/
    virtual Attribute *GetAttribute(uint64_t row, bool clone = true) const;
/*
~GetCount~ returns the number of entries in the attribut array.

*/
    virtual uint64_t GetCount() const;
/*
~GetSize~ returns the amount of space needed to save this attribut array
to persistant storage.

*/
    virtual uint64_t GetSize() const;

/*
~Save~ saves this attribut array
to persistant storage.

*/
    virtual void Save(CRelAlgebra::Writer &target, 
      bool includeHeader = true) const;

/*
~Append~ adds the entry at index ~row~ of the attribut array ~array~

*/
    virtual void Append(const CRelAlgebra::AttrArray & array, uint64_t row);
/*
~Append~ adds the row orientied attribute ~value~

*/
    virtual void Append(Attribute & value);
/*
~Remove~ removes the last added entry

*/
    virtual void Remove();
/*
~Clear~ removes all entries

*/
    virtual void Clear();

/*
~IsDefined~ returns true, iff the entry with index ~row~ is defined

*/
    virtual bool IsDefined(uint64_t row) const;

/*
~Compare~ compares the entry at index ~rowA~ with the entry at index
~rowB~ in ~arrayB~

*/
    virtual int  Compare(uint64_t rowA, const AttrArray& arrayB, 
      uint64_t rowB) const;
/*
~Compare~ compares the entry at index ~rowA~ with the row oriented
attribute ~value~

*/
    virtual int  Compare(uint64_t row, Attribute &value) const;

/*
~GetHash~ returns a hash value for the entry at index ~row~

*/
    virtual uint64_t GetHash(uint64_t row) const;

/*
1.1.2 Operators

The following functions implement the operators supported by the attribut
array.

~atInstant~ is a timeslice operator and computes an intime for moving object
entry in the attribute array and adds them to ~result~

*/
    void atInstant(Instant instant, typename Unit::Instants & result);
/*
~atPeriods~ restricts the moving object entries to a given set of time 
intervals and adds the resulting units to ~result~.

*/
    void atPeriods(temporalalgebra::Periods periods, MFsObjects & result);
/*
~passes~ adds the indices of all moving object entries to ~result~, which
ever assume the specified value or a value in the specified range. 

*/
    void passes(typename Unit::Attr & value, CRelAlgebra::LongInts & result);
    void passes(typename Unit::RAttr & value, CRelAlgebra::LongInts & result);
/*
~at~ restricts the all moving object entries to the specified value or 
the specified range. the computed new units are added to ~result~

*/
    void at(typename Unit::Attr & value, MFsObjects & result);
    void at(typename Unit::RAttr & value, MFsObjects & result);
/*
~addRandomUnits~ adds random units to every moving object entry

*/
    void addRandomUnits(CcInt& size, MFsObjects & result);

/*
~addMObject~ adds a new moving object entry to the attribute array

*/
    void addMObject();
/*
~addUnit~ adds a new unit to the last added moving object entry

*/
    void addUnit(Unit & unit);

  protected:
/*
1.1.3 Attributes

~MObject~ represents a moving object entry. ~firstUnit~ is the index of the
first unit that belongs to the moving object in the array ~mUnits~.
~minimum~ and ~maximum~ are the extremes of the value of the moving object.

*/
    struct MObject { 
      int firstUnit; 
      typename Unit::Value minimum, maximum;
    };

/*
~mUnits~ represents the units of all moving object entries.

*/
    std::shared_ptr<Array<Unit>> m_Units;
/*
~mMObjects~ represents the moving object entries.

*/
    std::shared_ptr<Array<MObject>> m_MObjects;

/*
1.1.4 Helper Functions

The following functions are for convenience. They return the number of units,
the index of the first unit and one more than the last unit 
of a moving object entry and also the unit for a specified unit index.

*/
    int unitFirst(int mobject) const;
    int unitAfterLast(int mobject) const;
    int unitCount(int mobject) const;
    Unit & unit(int index) const;
  };

/*
1.2 Implementation of ~MFsObjects~

1.2.1 Constructors

These constructors are required by the crel algebra

*/
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
  MFsObjects<Unit>::MFsObjects(CRelAlgebra::Reader& source, uint64_t rowsCount)
  {
    m_Units = std::make_shared<Array<Unit>>(source);
    m_MObjects = std::make_shared<Array<MObject>>(source);
    m_DefTimes = std::make_shared<DefTimes>(source);
  }

  template<class Unit>
  MFsObjects<Unit>::MFsObjects(const MFsObjects &array,
    const CRelAlgebra::SharedArray<const uint64_t> &filter) :
    MObjects(array, filter),
    m_Units(array.m_Units),
    m_MObjects(array.m_MObjects)
  {
  }
  
/*
1.2.2 CRel Algebra Interface

the following functions are required by the crel algebra for all attribute 
arrays.

~Filter~ returns a duplicate of this attribut array with the speficied filter.

*/
  template<class Unit>
  CRelAlgebra::AttrArray* MFsObjects<Unit>::Filter(
    CRelAlgebra::SharedArray<const uint64_t> filter) const
  {
    return new MFsObjects<Unit>
      (*this, filter);
  }

/*
~GetAttribute~ converts the entry in ~row~ to a attribute for row oriented
relations.

*/
  template<class Unit>
  Attribute * MFsObjects<Unit>::GetAttribute(uint64_t row, bool clone) const
  {
    typename Unit::MAttr * mattr = new typename Unit::MAttr(unitCount(row));
    mattr->StartBulkLoad();

    for (int ui = unitFirst(row); ui < unitAfterLast(row); ui++)
      unit(ui).appendTo(*mattr);

    mattr->EndBulkLoad();
    return mattr;
  }

/*
~GetCount~ returns the number of entries in the attribut array.

*/
  template<class Unit>
  uint64_t MFsObjects<Unit>::GetCount() const
  {
    return m_MObjects->size();
  }

/*
~GetSize~ returns the amount of space needed to save this attribut array
to persistant storage.

*/
  template<class Unit>
  uint64_t MFsObjects<Unit>::
  GetSize() const
  {
    return  m_Units->savedSize() +
            m_MObjects->savedSize() +
            m_DefTimes->savedSize();
  }

/*
~Save~ saves this attribut array
to persistant storage.

*/
  template<class Unit>
  void MFsObjects<Unit>::Save(CRelAlgebra::Writer &target, 
    bool includeHeader) const
  {
    m_Units->save(target);
    m_MObjects->save(target);
    m_DefTimes->save(target);
  }

/*
~Append~ adds the entry at index ~row~ of the attribut array ~array~

*/
  template<class Unit>
  void MFsObjects<Unit>::Append(const CRelAlgebra::AttrArray & array, 
    uint64_t row)
  {
    const MFsObjects<Unit> & m = 
      static_cast<const MFsObjects<Unit> &>
      (array);

    addMObject();

    for (int ui = m.unitFirst(row); ui < m.unitAfterLast(row); ui++) 
      addUnit(m.unit(ui));
  }

/*
~Append~ adds the row orientied attribute ~value~

*/
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

/*
~Remove~ removes the last added entry

*/
  template<class Unit>
  void MFsObjects<Unit>::Remove()
  {
    MObject & m = m_MObjects->back();
    m_Units->resize(m.firstUnit);
    m_MObjects->pop_back();

    m_DefTimes->removeRow();
  }

/*
~Clear~ removes all entries

*/
  template<class Unit>
  void MFsObjects<Unit>::Clear()
  {
    m_Units->clear();
    m_MObjects->clear();

    m_DefTimes->clear();
  }

/*
~IsDefined~ returns true, iff the entry with index ~row~ is defined

*/
  template<class Unit>
  bool MFsObjects<Unit>::IsDefined(uint64_t row) const
  {
    return unitCount(row) > 0;
  }

/*
~Compare~ compares the entry at index ~rowA~ with the entry at index
~rowB~ in ~arrayB~

*/
  template<class Unit>
  int MFsObjects<Unit>::Compare(uint64_t rowA, 
    const CRelAlgebra::AttrArray &arrayB, uint64_t rowB) 
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

/*
~Compare~ compares the entry at index ~rowA~ with the row oriented
attribute ~value~

*/
  template<class Unit>
  int MFsObjects<Unit>::Compare(uint64_t row, Attribute &value) const
  {
    MFsObjects<Unit> mrs;
    mrs.Append(value);
    return Compare(row, mrs, 0);
  }

/*
~GetHash~ returns a hash value for the entry at index ~row~

*/
  template<class Unit>
  uint64_t MFsObjects<Unit>::
  GetHash(uint64_t row) const
  {
    if (m_Units->size() == 0)
      return 0;

    return (uint64_t)(unit(0).interval().s ^ unit(0).interval().e);
  }

/*
1.1.2 Operators

The following functions implement the operators supported by the attribut
array.

~atInstant~ is a timeslice operator and computes an intime for each 
moving object entry in the attribute array and adds them to ~result~. It does a
binary search on all units of each moving object entry and if it finds
a unit it calls the function ~atInstant~ for this unit to get the corresponding
intime.

*/
  template<class Unit>
  void MFsObjects<Unit>::atInstant(Instant instant, 
    typename Unit::Instants & result)
  {
    if (!instant.IsDefined()) {
      for (uint64_t i = 0; i < GetFilter().GetCount(); i++)
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

/*
~atPeriods~ restricts the moving object entries to a given set of time 
intervals and adds the resulting units to ~result~. It performs a parallel
scan of the provided time intervalls and all units of each moving object entry.
if an overlapping pair of a time interval and a unit definition interval is
found the intersection is calculated and used as the parameter for 
a call to ~restrictToInterval~ for the unit. The returned unit is added
to ~result~.

*/
  template<class Unit>
  void MFsObjects<Unit>::
  atPeriods(temporalalgebra::Periods periods, MFsObjects & result)
  {
    if (!periods.IsDefined() || periods.GetNoComponents() == 0) {
      for (uint64_t i = 0; i < GetFilter().GetCount(); i++)
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

/*
~passes~ adds the indices of all moving object entries to ~result~, which
ever assume the specified value. It checks the value against the minimum
and maximum of the moving object first and if the value is between, then 
iterates over all units of the moving object entry.

*/
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

/*
~passes~ adds the indices of all moving object entries to ~result~, which
ever assume a value in the specified range. 
It checks the range against the minimum
and maximum of the moving object first and in case of overlap, then 
iterates over all units of the moving object entry.

*/
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

/*
~at~ restricts the all moving object entries to the specified value. 
the computed new units are added to ~result~

*/
  template<class Unit>
  void MFsObjects<Unit>::at(typename Unit::Attr & value, MFsObjects & result)
  {
    if (!value.IsDefined()) {
      for (uint64_t i = 0; i < GetFilter().GetCount(); i++)
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

/*
~at~ restricts the all moving object entries to the specified 
range. the computed new units are added to ~result~

*/
  template<class Unit>
  void MFsObjects<Unit>::at(typename Unit::RAttr & value, MFsObjects & result)
  {
    if (!value.IsDefined()) {
      for (uint64_t i = 0; i < GetFilter().GetCount(); i++)
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


/*
~addMObject~ adds a new moving object entry to the attribute array

*/
  template<class Unit>
  void MFsObjects<Unit>::addMObject()
  {
    m_MObjects->emplace_back(MObject{ static_cast<int>(m_Units->size()) });
    m_DefTimes->addRow();
  }

/*
~addUnit~ adds a new unit to the last added moving object entry.
minimum and maximum of the moving object entry are updated.

*/
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
  
  
/*
~addRandomUnits~ adds random units to every moving object entry

*/
  template<class Unit>
  void MFsObjects<Unit>::addRandomUnits(CcInt& size, MFsObjects & result)
  {
    if (!size.IsDefined()) 
      return;
      
    int count = size.GetValue();
    
    for (auto & iterator : GetFilter()) {
      int row = iterator.GetRow();

      Interval interval {0, 0, true, false};

      if (!unitCount(row) == 0) {
        Unit& u = unit(unitAfterLast(row) - 1);
        interval.s = (static_cast<int64_t>(u.interval().e * 1.1) 
                     / MILLISECONDS + 1) * MILLISECONDS;
      }

      result.Append(*this, row);
      
      for (int i = 0; i < count; i++) {
        interval.e = interval.s + (5 + (rand() % 5)) * MILLISECONDS;
        Unit u = Unit::random(interval);
        result.addUnit(u);
        interval.s = interval.e;
      }
    }
  }

  
/*
1.2.4 Helper Functions

The following functions are for convenience. 

~unitFirst~ returns the index of the first unit of the moving object entry 
with index ~mobject~

*/
  template<class Unit>
  int MFsObjects<Unit>::unitFirst(int mobject) const
  {
    return (*m_MObjects)[mobject].firstUnit;
  }

/*
~unitAfterLast~ returns one more than the index of the last unit of 
the moving object entry 
with index ~mobject~

*/
  template<class Unit>
  int MFsObjects<Unit>::unitAfterLast(int mobject) const
  {
    return mobject < static_cast<int>(m_MObjects->size()) - 1 ? 
      (*m_MObjects)[mobject + 1].firstUnit : 
      m_Units->size();
  }

/*
~unitCount~ returns the number of units of the moving object entry 
with index ~mobject~

*/
  template<class Unit>
  int MFsObjects<Unit>::unitCount(int mobject) const
  {
    return unitAfterLast(mobject) - unitFirst(mobject);
  }

/*
~unit~ returns unit for a specified unit index

*/
  template<class Unit>
  Unit & MFsObjects<Unit>::unit(int index) const
  {
    return (*m_Units)[index];
  }

}
