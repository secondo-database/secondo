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
#include "MRegions.h"
#include "RegionTools.h"

using namespace std;

namespace temporalalgebra {
Word InMRegion(const ListExpr typeInfo,
  const ListExpr instance,
  const int errorPos,
  ListExpr& errorInfo,
  bool& correct);
}

extern NestedList* nl;

namespace ColumnMovingAlgebra
{
  CRelAlgebra::AttrArray* MRegions::Filter(
    CRelAlgebra::SharedArray<const size_t> filter) const
  {
    return new MRegions(*this, filter);
  }

  size_t MRegions::GetCount() const
  {
    return m_MRegions->size();
  }

  size_t MRegions::GetSize() const
  {
    return  m_Edges->savedSize() +
            m_Cycles->savedSize() +
            m_Faces->savedSize() +
            m_Units->savedSize() +
            m_MRegions->savedSize() +
            m_DefTimes->savedSize();
  }

  void MRegions::Save(CRelAlgebra::Writer &target, bool includeHeader) const
  {
    m_Edges->save(target);
    m_Cycles->save(target);
    m_Faces->save(target);
    m_Units->save(target);
    m_MRegions->save(target);
    m_DefTimes->save(target);
  }

  void MRegions::Append(const CRelAlgebra::AttrArray & array, size_t row)
  {
    const MRegions & mrs = static_cast<const MRegions &>(array);

    addMRegion();

    for (int ui = mrs.unitFirst(row); ui < mrs.unitAfterLast(row); ui++) {
      addUnit(mrs.unit(ui).interval);

      for (int fi = mrs.faceFirst(ui); fi < mrs.faceAfterLast(ui); fi++) {
        addFace();

        for (int ci = mrs.cycleFirst(fi); ci < mrs.cycleAfterLast(fi); ci++) {
          addCycle();

          for (int ei = mrs.edgeFirst(ci); ei < mrs.edgeAfterLast(ci); ei++) 
            addEdge(mrs.edge(ei));
        }
      }
    }
  }

  void MRegions::Append(Attribute & value)
  {
    auto & m = static_cast<temporalalgebra::MRegion&>(value);
    auto segments = static_cast<DbArray<temporalalgebra::MSegmentData>*>(
      m.GetFLOB(1));

    addMRegion();

    if (!m.IsDefined())
      return;

    for (int ui = 0; ui < m.GetNoComponents(); ui++) {
      temporalalgebra::URegionEmb u;
      m.Get(ui, u);

      addUnit(u.getTimeInterval());

      int num = u.GetSegmentsNum(), lastFace = -1, lastCycle = -1;

      for (int i = 0; i < num; i++) {
        temporalalgebra::MSegmentData dms;
        u.GetSegment(segments, i, dms);

        if (lastFace != static_cast<int>(dms.GetFaceNo())) {
          addFace();
          lastFace = dms.GetFaceNo();
          lastCycle = -1;
        }

        if (lastCycle != static_cast<int>(dms.GetCycleNo())) {
          addCycle();
          lastCycle = dms.GetCycleNo();
        }

        addEdge(Edge{ { dms.GetInitialStartX(), dms.GetInitialStartY() },
                      { dms.GetFinalStartX(),   dms.GetFinalStartY()   } });
      }
    }
  }

  void MRegions::Remove()
  {
    MRegion & r = m_MRegions->back();
    Unit & u = unit(r.firstUnit);
    Face & f = face(u.firstFace);
    Cycle & c = cycle(f.firstCycle);

    m_Edges->resize(c.firstEdge);
    m_Cycles->resize(f.firstCycle);
    m_Faces->resize(u.firstFace);
    m_Units->resize(r.firstUnit);
    m_MRegions->pop_back();

    m_DefTimes->removeRow();
  }

  void MRegions::Clear()
  {
    m_Edges->clear();
    m_Cycles->clear();
    m_Faces->clear();
    m_Units->clear();
    m_MRegions->clear();

    m_DefTimes->clear();
  }

  bool MRegions::IsDefined(size_t row) const
  {
    return unitCount(row) > 0;
  }

  int MRegions::Compare(size_t rowA, const CRelAlgebra::AttrArray &arrayB,
    size_t rowB) const
  {
    const MRegions & mpointsB = static_cast<const MRegions&>(arrayB);

    int iDiff = unitCount(rowA) - mpointsB.unitCount(rowB);
    if (iDiff != 0)
      return iDiff < 0 ? -1 : 1;

    for (int uiA = unitFirst(rowA), uiB = mpointsB.unitFirst(rowB); 
         uiA < unitAfterLast(rowA); uiA++, uiB++) 
    {
      iDiff = unit(uiA).interval.compare(mpointsB.unit(uiB).interval);
      if (iDiff != 0)
        return iDiff < 0 ? -1 : 1;

      iDiff = faceCount(uiA) - mpointsB.faceCount(uiB);
      if (iDiff != 0)
        return iDiff < 0 ? -1 : 1;

      for (int fiA = faceFirst(uiA), fiB = mpointsB.faceFirst(uiB); 
           fiA < faceAfterLast(uiA); fiA++, fiB++) 
      {
        iDiff = cycleCount(fiA) - mpointsB.cycleCount(fiB);
        if (iDiff != 0)
          return iDiff < 0 ? -1 : 1;

        for (int ciA = cycleFirst(fiA), ciB = mpointsB.cycleFirst(fiB); 
             ciA < cycleAfterLast(fiA); ciA++, ciB++) 
        {
          iDiff = edgeCount(ciA) - mpointsB.edgeCount(ciB);
          if (iDiff != 0)
            return iDiff < 0 ? -1 : 1;

          for (int eiA = edgeFirst(ciA), eiB = mpointsB.edgeFirst(ciB); 
               eiA < edgeAfterLast(ciA); eiA++, eiB++) 
          {
            double dDiff;

            dDiff = edge(eiA).s.x - mpointsB.edge(eiB).s.x;
            if (dDiff != 0)
              return dDiff < 0 ? -1 : 1;

            dDiff = edge(eiA).s.y - mpointsB.edge(eiB).s.y;
            if (dDiff != 0)
              return dDiff < 0 ? -1 : 1;

            dDiff = edge(eiA).e.x - mpointsB.edge(eiB).e.x;
            if (dDiff != 0)
              return dDiff < 0 ? -1 : 1;

            dDiff = edge(eiA).e.y - mpointsB.edge(eiB).e.y;
            if (dDiff != 0)
              return dDiff < 0 ? -1 : 1;
          }
        }
      }
    }

    return 0;
  }

  int MRegions::Compare(size_t row, Attribute &value) const
  {
    MRegions mrs;
    mrs.Append(value);
    return Compare(row, mrs, 0);
  }

  int MRegions::CompareAlmost(size_t rowA, const CRelAlgebra::AttrArray 
    &arrayB, size_t rowB) const
  {
    return Compare(rowA, arrayB, rowB);
  }

  int MRegions::CompareAlmost(size_t row, Attribute &value) const
  {
    return Compare(row, value);
  }

  bool MRegions::Equals(size_t rowA, const CRelAlgebra::AttrArray &arrayB,
    size_t rowB) const
  {
    return Compare(rowA, arrayB, rowB) == 0;
  }

  bool MRegions::Equals(size_t row, Attribute &value) const
  {
    return Compare(row, value) == 0;
  }

  bool MRegions::EqualsAlmost(size_t rowA, 
    const CRelAlgebra::AttrArray &arrayB, size_t rowB) const
  {
    return Compare(rowA, arrayB, rowB) == 0;
  }

  bool MRegions::EqualsAlmost(size_t row, Attribute &value) const
  {
    return Compare(row, value) == 0;
  }

  size_t MRegions::GetHash(size_t row) const
  {
    if (m_Units->size() == 0)
      return 0;

    return (size_t)(unit(0).interval.s ^ unit(0).interval.e);
  }


  Attribute * MRegions::GetAttribute(size_t row, bool clone) const
  {
    ListExpr mrsll, mrsl = nl->TheEmptyList();

    for (int ui = unitFirst(row); ui < unitAfterLast(row); ui++) {
      ListExpr rll, rl = nl->TheEmptyList();

      for (int fi = faceFirst(ui); fi < faceAfterLast(ui); fi++) {
        ListExpr fll, fl = nl->TheEmptyList();

        for (int ci = cycleFirst(fi); ci < cycleAfterLast(fi); ci++) {
          ListExpr cll, cl = nl->TheEmptyList();

          for (int ei = edgeFirst(ci); ei < edgeAfterLast(ci); ei++) {
            Point & s = edge(ei).s, &e = edge(ei).e;
            ListExpr p = nl->FourElemList(nl->RealAtom(s.x), nl->RealAtom(s.y),
              nl->RealAtom(e.x), nl->RealAtom(e.y));

            if (cl == nl->TheEmptyList())
              cll = cl = nl->OneElemList(p);
            else
              cll = nl->Append(cll, p);
          }

          if (fl == nl->TheEmptyList())
            fll = fl = nl->OneElemList(cl);
          else
            fll = nl->Append(fll, cl);
        }

        if (rl == nl->TheEmptyList())
          rll = rl = nl->OneElemList(fl);
        else
          rll = nl->Append(rll, fl);
      }

      temporalalgebra::Interval<Instant> interval = 
        unit(ui).interval.convert();
        
      ListExpr ul = nl->TwoElemList(nl->FourElemList(
        nl->StringAtom(interval.start.ToString()),
        nl->StringAtom(interval.end.ToString()),
        nl->BoolAtom(interval.lc),
        nl->BoolAtom(interval.rc)),
        rl);

      if (mrsl == nl->TheEmptyList())
        mrsll = mrsl = nl->OneElemList(ul);
      else
        mrsll = nl->Append(mrsll, ul);
    }

    ListExpr e;
    bool c;
    return static_cast<Attribute*>(
      temporalalgebra::InMRegion(nl->TheEmptyList(), mrsl, 0, e, c).addr);
  }

  void MRegions::present(Instant instant, CRelAlgebra::LongInts & result)
  {
    result.Clear();
    
    for (auto & iterator : GetFilter()) {
      int i = iterator.GetRow();
      
      if (m_DefTimes->present(i, instant))
        result.Append(i);
    }
  }

  void MRegions::present(temporalalgebra::Periods periods, 
    CRelAlgebra::LongInts & result)
  {
    result.Clear();

    for (auto & iterator : GetFilter()) {
      int i = iterator.GetRow();
      
      if (m_DefTimes->present(i, periods))
        result.Append(i);
    }
  }

  void MRegions::atInstant(Instant instant, IRegions & result)
  {
    temporalalgebra::IRegion undefined(false);
    undefined.SetDefined(false);

    if (!instant.IsDefined()) {
      for (size_t i = 0; i < GetFilter().GetCount(); i++)
        result.Append(undefined);

      return;
    }

    int64_t time = instant.millisecondsToNull();
    

    for (auto & iterator : GetFilter()) {
      int i = iterator.GetRow();

      Unit * first = m_Units->data() + unitFirst(i);
      Unit * afterLast = m_Units->data() + unitAfterLast(i);
      Unit * u = lower_bound(first, afterLast, time,
        [] (const Unit & a, const int64_t & b) -> bool { 
          return a.interval.e < b || (a.interval.e == b && !a.interval.rc); 
        });

      if ( u == afterLast || u->interval.s > time || 
          (u->interval.s == time && !u->interval.lc)) {
        result.Append(undefined);
        continue;
      }

      Interval & iv = u->interval;
      double d = iv.e - iv.s;
      double f = d == 0.0 ? 1.0 : (time - iv.s) / d;
      
      vector<vector<::Point>> cycles;

      int afterLastFace = u == &m_Units->back() ? 
        m_Faces->size() : 
        (u + 1)->firstFace;
        
      for (int fi = u->firstFace; fi < afterLastFace; fi++) {
        for (int ci = cycleFirst(fi); ci < cycleAfterLast(fi); ci++) {
          cycles.emplace_back();

          for (int ei = edgeFirst(ci); ei < edgeAfterLast(ci); ei++) {
            Edge & e = edge(ei);
            double x = (1.0 - f) * e.s.x + f * e.e.x;
            double y = (1.0 - f) * e.s.y + f * e.e.y;
            cycles.back().emplace_back(::Point(true, x, y));
          }
          
          if (cycles.back().size() > 0)
            cycles.back().push_back(cycles.back().front());

          if ((ci == cycleFirst(fi)) != getDir(cycles.back()))
            reverseCycle(cycles.back());
        }
      }

      Region * r = buildRegion(cycles);
      auto ir = temporalalgebra::IRegion(time, *r);
      result.Append(ir);
      delete r;
    }
  }

  void MRegions::atPeriods(temporalalgebra::Periods periods, MRegions & result)
  {
    if (!periods.IsDefined() || periods.GetNoComponents() == 0) {
      for (size_t i = 0; i < GetFilter().GetCount(); i++)
        result.addMRegion();

      return;
    }
    
    for (auto & iterator : GetFilter()) {
      int row = iterator.GetRow();
      result.addMRegion();

      int ai = 0, bi = unitFirst(row);

      while (ai < periods.GetNoComponents() && bi < unitAfterLast(row)) {
        temporalalgebra::Interval<Instant> period;
        periods.Get(ai, period);
        Interval a = period;
        Interval & b = unit(bi).interval;

        if (a.intersects(b)) {
          Interval c = a.intersection(b);
          result.addUnit(c);

          double d = b.e - b.s;
          double fs = d == 0.0 ? 1.0 : (c.s - b.s) / d;
          double fe = d == 0.0 ? 1.0 : (c.e - b.s) / d;

          for (int fi = faceFirst(bi); fi < faceAfterLast(bi); fi++) {
            result.addFace();

            for (int ci = cycleFirst(fi); ci < cycleAfterLast(fi); ci++) {
              result.addCycle();

              for (int ei = edgeFirst(ci); ei < edgeAfterLast(ci); ei++) {
                Edge & e = edge(ei);
                Edge f;
                f.s.x = (1.0 - fs) * e.s.x + fs * e.e.x;
                f.s.y = (1.0 - fs) * e.s.y + fs * e.e.y;
                f.e.x = (1.0 - fe) * e.s.x + fe * e.e.x;
                f.e.y = (1.0 - fe) * e.s.y + fe * e.e.y;
                result.addEdge(f);
              }
            }
          }
        }

        if (a.e < b.e || (a.e == b.e && b.rc))
          ai++;
        else
          bi++;
      }
    }
  }

}
