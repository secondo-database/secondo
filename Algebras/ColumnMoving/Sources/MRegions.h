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
#include "MObjects.h"
#include "MPoints.h"
#include "IRegions.h"
#include "RegionTools.h"

namespace ColumnMovingAlgebra
{
  class MRegions : public MObjects
  {
  public:
    struct Vec2 {
      double x, y;
      bool almostEqual(const Vec2 & b) const;
      Vec2 operator - (const Vec2 & b) const;
      Vec2 operator + (const Vec2 & b) const;
    };
    struct Edge { Vec2 s, e; };

    MRegions();
    MRegions(CRelAlgebra::Reader& source);
    MRegions(CRelAlgebra::Reader& source, size_t rowsCount);
    MRegions(const MRegions &array, 
             const CRelAlgebra::SharedArray<const size_t> &filter);
    virtual ~MRegions() { }

    virtual AttrArray* Filter(
      CRelAlgebra::SharedArray<const size_t> filter) const;

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
    virtual int  Compare(size_t row, Attribute &value) const;

    virtual size_t GetHash(size_t row) const;

    void atInstant(Instant instant, IRegions & result);
    void atPeriods(temporalalgebra::Periods periods, MRegions & result);
    void intersection(MPoints mpoints, MPoints & result);

    void addMRegion();
    void addUnit(Interval interval);
    void addFace();
    void addCycle();
    void addEdge(Edge edge);

    void addConstMRegion(Region & region, Interval interval);

    void checkMRegion(int index);
    
  private:
    static const double SMALL_NUM;

    static const int ENTRY_POINT = 0, EXIT_POINT = 1, UNKNOWN = 2;
    static const int LEFT_CLOSED = 3, LEFT_OPEN = 4, 
      RIGHT_CLOSED = 5, RIGHT_OPEN = 6;
    static const int INSIDE = 0, BOUNDARY = 1, OUTSIDE = 2;

    struct Cycle { int firstEdge; };
    struct Face { int firstCycle; };
    struct Unit { Interval interval; int firstFace; };
    struct MRegion { int firstUnit; };

    std::shared_ptr<Array<Edge>> m_Edges;
    std::shared_ptr<Array<Cycle>> m_Cycles;
    std::shared_ptr<Array<Face>> m_Faces;
    std::shared_ptr<Array<Unit>> m_Units;
    std::shared_ptr<Array<MRegion>> m_MRegions;

    int unitAfterLast(int mregion) const;
    int faceAfterLast(int unit) const;
    int cycleAfterLast(int face) const;
    int edgeAfterLast(int cycle) const;
    int unitFirst(int mregion) const;
    int faceFirst(int unit) const;
    int cycleFirst(int face) const;
    int edgeFirst(int cycle) const;
    int unitCount(int mregion) const;
    int faceCount(int unit) const;
    int cycleCount(int face) const;
    int edgeCount(int cycle) const;
    Unit & unit(int index) const;
    Face & face(int index) const;
    Cycle & cycle(int index) const;
    Edge & edge(int index) const;

    struct Vec3 {
      double x, y, t;
      Vec3 operator-(const Vec3 & b) const;
      double dot(const Vec3 & b) const;
      Vec3 cross(const Vec3 & b) const;
    };

    struct Hit { 
      double position; 
      int type; 
      int edge;
      bool operator < (const Hit & b) const;
    };

    struct MPointUnit {
      Interval interval;
      Vec2 s, e;
    };

    bool cycleIsClockwise(int cycle);
    void cycleReverse(int cycle);

    void intersection(int mregionUnit, MPointsData::Unit mpointUnit,
      std::list<MPointsData::Unit> & result);
    void calculateHits(int mregionUnit, Vec2 orig, Vec2 dir, double iS, 
      double iL, std::set<Hit> & result);
    bool calculateHit(const Vec3 & orig, const Vec3 & dir,
      const Vec3 & v0, const Vec3 & v1, const Vec3 & v2,
      Hit & result);
    void combineNearHits(std::set<Hit> & hits, std::list<Hit> & result);
    bool cleanHits(std::set<Hit> & hits, std::list<Hit> & result,
      double start, double end, bool lc, bool rc);
    bool deduceHitTypes(std::list<Hit> hits, bool forward, 
      std::list<Hit> & result);
    int64_t bestTimeForRelation(int mregionUnit, double pIS, double pIL,
      std::list<Hit> hits);
    int relation(int mregionUnit, MPointsData::Unit mpointUnit, int64_t time);
    void addMPointUnit(Vec2 p0, Vec2 p1, double pIS, double pIE, int id,
      double start, double end, bool lc, bool rc, 
      std::list<MPointsData::Unit> & result);
  };

  inline MRegions::MRegions() :
    m_Edges(std::make_shared<Array<Edge>>()),
    m_Cycles(std::make_shared<Array<Cycle>>()),
    m_Faces(std::make_shared<Array<Face>>()),
    m_Units(std::make_shared<Array<Unit>>()),
    m_MRegions(std::make_shared<Array<MRegion>>())
  {
  }

  inline MRegions::MRegions(CRelAlgebra::Reader& source)
  {
    m_Edges = std::make_shared<Array<Edge>>(source);
    m_Cycles = std::make_shared<Array<Cycle>>(source);
    m_Faces = std::make_shared<Array<Face>>(source);
    m_Units = std::make_shared<Array<Unit>>(source);
    m_MRegions = std::make_shared<Array<MRegion>>(source);
    m_DefTimes = std::make_shared<DefTimes>(source);
  }

  inline MRegions::MRegions(CRelAlgebra::Reader& source, size_t rowsCount)
  {
    m_Edges = std::make_shared<Array<Edge>>(source);
    m_Cycles = std::make_shared<Array<Cycle>>(source);
    m_Faces = std::make_shared<Array<Face>>(source);
    m_Units = std::make_shared<Array<Unit>>(source);
    m_MRegions = std::make_shared<Array<MRegion>>(source);
    m_DefTimes = std::make_shared<DefTimes>(source);
  }

  inline MRegions::MRegions(const MRegions &array, 
    const CRelAlgebra::SharedArray<const size_t> &filter) :
    MObjects(array, filter),
    m_Edges(array.m_Edges),
    m_Cycles(array.m_Cycles),
    m_Faces(array.m_Faces),
    m_Units(array.m_Units),
    m_MRegions(array.m_MRegions)
  {
  }
  
  inline void MRegions::atInstant(Instant instant, IRegions & result)
  {
    temporalalgebra::IRegion undefined(false);
    undefined.SetDefined(false);

    if (!instant.IsDefined()) {

      for (size_t i = 0; i < GetFilter().GetCount(); i++) 
        result.Append(undefined);
        
      return;
    }
    
    int64_t time = instant.millisecondsToNull();

    if (!instant.IsDefined()) {
      for (size_t i = 0; i < GetFilter().GetCount(); i++)
        result.Append(undefined);

      return;
    }

    for (auto & iterator : GetFilter()) {
      int i = iterator.GetRow();

      Unit * first = m_Units->data() + unitFirst(i);
      Unit * afterLast = m_Units->data() + unitAfterLast(i);
      Unit * u = std::lower_bound(first, afterLast, time,
        [] (const Unit & a, const int64_t & b) -> bool { 
          return a.interval.e < b || (a.interval.e == b && !a.interval.rc); 
        });

      if ( u == afterLast || u->interval.s > time || 
          (u->interval.s == time && !u->interval.lc)) {
        result.Append(undefined);
        continue;
      }

      Interval & iv = u->interval;
      double d = static_cast<double>(iv.e - iv.s);
      double f = d == 0.0 ? 1.0 : (time - iv.s) / d;
      
      std::vector<std::vector<::Point>> cycles;

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

      Region * r = buildRegion2(cycles);
      auto ir = temporalalgebra::IRegion(time, *r);
      result.Append(ir);
      delete r;
    }
  }

  inline void MRegions::atPeriods(temporalalgebra::Periods periods, 
    MRegions & result)
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

          double d = static_cast<double>(b.e - b.s);
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

  inline void MRegions::intersection(MPoints mpoints, MPoints & result)
  {
    CRelAlgebra::AttrArrayFilter rf = GetFilter(), pf = mpoints.GetFilter();
    size_t rc = rf.GetCount(), pc = pf.GetCount();
    check(rc == 1 || pc == 1 || rc == pc, 
          "the mregions and mpoints operant have a different row count");

    int count = std::max(rc, pc);

    for (int i = 0; i < count; i++) {
      int ri = rf.GetAt(i % rc);
      int pi = pf.GetAt(i % pc);

      std::list<MPointsData::Unit> resultUnits;

      int rUnitIndex = unitFirst(ri);
      MPointsData::UnitIterator pUnitIterator = mpoints.unitIterator(pi);
      bool finished = rUnitIndex >= unitAfterLast(ri) || 
                      !pUnitIterator.hasNext();

      Unit rUnit;
      MPointsData::Unit pUnit;
      if (!finished) {
        rUnit = unit(rUnitIndex);
        pUnit = pUnitIterator.next();
      }

      while (!finished) {
        intersection(rUnitIndex, pUnit, resultUnits);

        if ( rUnit.interval.e <  pUnit.interval.e ||
            (rUnit.interval.e == pUnit.interval.e && pUnit.interval.rc) ) 
        {
          finished = ++rUnitIndex >= unitAfterLast(ri);
          if (!finished)
            rUnit = unit(rUnitIndex);
        } else {
          finished = !pUnitIterator.hasNext();
          if (!finished)
            pUnit = pUnitIterator.next();
        }
      }

      result.addRow();
      for (auto & u : resultUnits)
        result.addUnit(u.interval, u.x0, u.y0, u.x1, u.y1);
    }
  }

  inline void MRegions::addMRegion()
  {
    m_MRegions->emplace_back(MRegion{ static_cast<int>(m_Units->size()) });
    m_DefTimes->addRow();
  }

  inline bool MRegions::Vec2::almostEqual(const Vec2 & b) const
  {
    return std::abs(x - b.x) < SMALL_NUM && std::abs(y - b.y) < SMALL_NUM;
  }

  inline MRegions::Vec2 MRegions::Vec2::operator-(const Vec2 & b) const
  {
    return { x - b.x, y - b.y };
  }

  inline MRegions::Vec2 MRegions::Vec2::operator+(const Vec2 & b) const
  {
    return { x + b.x, y + b.y };
  }
  
  inline void MRegions::addUnit(Interval interval)
  {
    m_Units->emplace_back(Unit{ interval, 
                                static_cast<int>(m_Faces->size()) });
    m_DefTimes->addInterval(interval);
  }
  
  inline void MRegions::addFace()
  {
    m_Faces->emplace_back(Face{ static_cast<int>(m_Cycles->size()) });
  }
  
  inline void MRegions::addCycle()
  {
    m_Cycles->emplace_back(Cycle{ static_cast<int>(m_Edges->size()) });
  }
  
  inline void MRegions::addEdge(Edge edge)
  {
    m_Edges->push_back(edge);
  }
  
  inline void MRegions::addConstMRegion(Region & region, Interval interval)
  {
    addMRegion();
    
    if(!region.IsDefined() || region.IsEmpty())
      return;
      
    addUnit(interval);
    addFace();
    addCycle();

    Region *RCopy=new Region(region, true); // in memory

    RCopy->LogicSort();

    HalfSegment hs, hsnext;

    int currFace = -999999, currCycle= -999999; // avoid uninitialized use
    Point outputP, leftoverP;

    for( int i = 0; i < RCopy->Size(); i++ )
    {
      RCopy->Get( i, hs );
      if (i==0)
      {
        currFace = hs.attr.faceno;
        currCycle = hs.attr.cycleno;
        RCopy->Get( i+1, hsnext );

        if ((hs.GetLeftPoint() == hsnext.GetLeftPoint()) ||
            ((hs.GetLeftPoint() == hsnext.GetRightPoint())))
        {
          outputP = hs.GetRightPoint();
          leftoverP = hs.GetLeftPoint();
        }
        else if ((hs.GetRightPoint() == hsnext.GetLeftPoint()) ||
                 ((hs.GetRightPoint() == hsnext.GetRightPoint())))
        {
          outputP = hs.GetLeftPoint();
          leftoverP = hs.GetRightPoint();
        }
        else
        {
          check(false, "Wrong data format: discontiguous segments");
        }
        
        addEdge(Edge{{outputP.GetX(), outputP.GetY()},
                                        {outputP.GetX(), outputP.GetY()}});
      }
      else
      {
        if (hs.attr.faceno == currFace)
        {
          if (hs.attr.cycleno == currCycle)
          {
            outputP=leftoverP;

            if (hs.GetLeftPoint() == leftoverP)
              leftoverP = hs.GetRightPoint();
            else if (hs.GetRightPoint() == leftoverP)
            {
              leftoverP = hs.GetLeftPoint();
            }
            else
            {
              check(false, "Wrong data format: discontiguous segments");
            }

            addEdge(Edge{{outputP.GetX(), outputP.GetY()},
                                            {outputP.GetX(), outputP.GetY()}});
          }
          else
          {
            addCycle();
            currCycle = hs.attr.cycleno;

            RCopy->Get( i+1, hsnext );
            if ((hs.GetLeftPoint() == hsnext.GetLeftPoint()) ||
                ((hs.GetLeftPoint() == hsnext.GetRightPoint())))
            {
              outputP = hs.GetRightPoint();
              leftoverP = hs.GetLeftPoint();
            }
            else if ((hs.GetRightPoint() == hsnext.GetLeftPoint()) ||
                     ((hs.GetRightPoint() == hsnext.GetRightPoint())))
            {
              outputP = hs.GetLeftPoint();
              leftoverP = hs.GetRightPoint();
            }
            else
            {
              check(false, "Wrong data format: discontiguous segments");
            }

            addEdge(Edge{{outputP.GetX(), outputP.GetY()},
                                            {outputP.GetX(), outputP.GetY()}});
          }
        }
        else
        {
          addFace();
          addCycle();
          currFace = hs.attr.faceno;
          currCycle = hs.attr.cycleno;

          RCopy->Get( i+1, hsnext );
          if ((hs.GetLeftPoint() == hsnext.GetLeftPoint()) ||
             ((hs.GetLeftPoint() == hsnext.GetRightPoint())))
          {
            outputP = hs.GetRightPoint();
            leftoverP = hs.GetLeftPoint();
          }
          else if ((hs.GetRightPoint() == hsnext.GetLeftPoint()) ||
                  ((hs.GetRightPoint() == hsnext.GetRightPoint())))
          {
            outputP = hs.GetLeftPoint();
            leftoverP = hs.GetRightPoint();
          }
          else
          {
            check(false, "Wrong data format: discontiguous segments");
          }

          addEdge(Edge{{outputP.GetX(), outputP.GetY()},
                                          {outputP.GetX(), outputP.GetY()}});
        }
      }
    }
    
    checkMRegion(0);
    RCopy->DeleteIfAllowed();
  }

  inline void MRegions::checkMRegion(int index)
  {
    for (int ui = unitFirst(index); ui < unitAfterLast(index); ui++) {
      for (int fi = faceFirst(ui); fi < faceAfterLast(ui); fi++) {
        for (int ci = cycleFirst(fi); ci < cycleAfterLast(fi); ci++) {
          bool isHole = ci == cycleFirst(fi);
          if (cycleIsClockwise(ci) == isHole)
            cycleReverse(ci);
        }
      }
    }
  }
  
  inline int MRegions::unitAfterLast(int mregion) const
  {
    return mregion < static_cast<int>(m_MRegions->size()) - 1 ? 
      (*m_MRegions)[mregion + 1].firstUnit : 
      m_Units->size();
  }

  inline int MRegions::faceAfterLast(int unit) const
  {
    return unit < static_cast<int>(m_Units->size()) - 1 ? 
      (*m_Units)[unit + 1].firstFace : 
      m_Faces->size();
  }

  inline int MRegions::cycleAfterLast(int face) const
  {
    return face < static_cast<int>(m_Faces->size()) - 1 ? 
      (*m_Faces)[face + 1].firstCycle : 
      m_Cycles->size();
  }

  inline int MRegions::edgeAfterLast(int cycle) const
  {
    return cycle < static_cast<int>(m_Cycles->size()) - 1 ? 
      (*m_Cycles)[cycle + 1].firstEdge : 
      m_Edges->size();
  }

  inline int MRegions::unitFirst(int mregion) const
  {
    return (*m_MRegions)[mregion].firstUnit;
  }

  inline int MRegions::faceFirst(int unit) const
  {
    return (*m_Units)[unit].firstFace;
  }

  inline int MRegions::cycleFirst(int face) const
  {
    return (*m_Faces)[face].firstCycle;
  }

  inline int MRegions::edgeFirst(int cycle) const
  {
    return (*m_Cycles)[cycle].firstEdge;
  }

  inline int MRegions::unitCount(int mregion) const
  {
    return unitAfterLast(mregion) - unitFirst(mregion);
  }

  inline int MRegions::faceCount(int unit) const
  {
    return faceAfterLast(unit) - faceFirst(unit);
  }

  inline int MRegions::cycleCount(int face) const
  {
    return cycleAfterLast(face) - cycleFirst(face);
  }

  inline int MRegions::edgeCount(int cycle) const
  {
    return edgeAfterLast(cycle) - edgeFirst(cycle);
  }
  
  inline MRegions::Unit & MRegions::unit(int index) const
  {
    return (*m_Units)[index];
  }

  inline MRegions::Face & MRegions::face(int index) const
  {
    return (*m_Faces)[index];
  }

  inline MRegions::Cycle & MRegions::cycle(int index) const
  {
    return (*m_Cycles)[index];
  }

  inline MRegions::Edge & MRegions::edge(int index) const
  {
    return (*m_Edges)[index];
  }

  inline MRegions::Vec3 MRegions::Vec3::operator-(const Vec3 & b) const
  {
    return { x - b.x, y - b.y, t - b.t };
  }

  inline double MRegions::Vec3::dot(const Vec3 & b) const
  {
    return x * b.x + y * b.y + t * b.t;
  }

  inline MRegions::Vec3 MRegions::Vec3::cross(const Vec3 & b) const
  {
    return { y * b.t - t * b.y, t * b.x - x * b.t, x * b.y - y * b.x };
  }

  inline bool MRegions::Hit::operator<(const Hit & b) const
  {
    return position > b.position || (position == b.position && edge > b.edge);
  }

  inline bool MRegions::cycleIsClockwise(int cycle)
  {
    double area = 0.0;

    for (int ei = edgeFirst(cycle); ei < edgeAfterLast(cycle); ei++) {
      Edge & e0 = edge(ei);
      Edge & e1 = ei < edgeAfterLast(cycle) - 1 ?
        edge(ei + 1) :
        edge(edgeFirst(cycle));

      Vec2 p0{ e0.s.x + e0.e.x, e0.s.y + e0.e.y };
      Vec2 p1{ e1.s.x + e1.e.x, e1.s.y + e1.e.y };

      area += (p1.x - p0.x) * (p1.y + p0.y);
    }

    return area > 0.0;
  }

  inline void MRegions::cycleReverse(int cycle)
  {
    for (int ei0 = edgeFirst(cycle), ei1 = edgeAfterLast(cycle) - 1; 
         ei0 < ei1; ei0++, ei1--) 
      std::swap(edge(ei0), edge(ei1));
  }

  inline void MRegions::intersection(int mregionUnit, 
    MPointsData::Unit mpointUnit, std::list<MPointsData::Unit> & result)
  {
    Interval & rI = unit(mregionUnit).interval;
    Interval & pI = mpointUnit.interval;

    if (!rI.intersects(pI))
      return;

    Interval interval = rI.intersection(pI);

    Vec2 p0 = Vec2{ mpointUnit.x0, mpointUnit.y0 };
    Vec2 p1 = Vec2{ mpointUnit.x1, mpointUnit.y1 };
    double pIS = static_cast<double>(mpointUnit.interval.s);
    double pIE = static_cast<double>(mpointUnit.interval.e);
    double pIL = pIE - pIS, s, e;

    if (pIL <= SMALL_NUM) {
      s = e = 0.0;
      pIL = 1.0;
    } else {
      s = (interval.s - pI.s) / pIL;
      e = (interval.e - pI.s) / pIL;
    }

    std::set<Hit> hits;
    calculateHits(mregionUnit, p0, p1 - p0, pIS, pIL, hits);
    std::list<Hit> cleaned;
    bool hitTypesKnown = cleanHits(hits, cleaned, s, e, 
                                   interval.lc, interval.rc);

    if (hitTypesKnown) {
      double p = 0.0;
      bool lc;

      for (auto & h : cleaned)
        if (h.type == LEFT_CLOSED || h.type == LEFT_OPEN) {
          p = h.position;
          lc = h.type == LEFT_CLOSED;
        } else {
          addMPointUnit(p0, p1, pIS, pIE, mpointUnit.id,
            p, h.position, lc, h.type == RIGHT_CLOSED, result);
        }
    } else {
      int64_t t = bestTimeForRelation(mregionUnit, pIS, pIE, cleaned);

      if (relation(mregionUnit, mpointUnit, t) != OUTSIDE) {
        addMPointUnit(p0, p1, pIS, pIE, mpointUnit.id, 
          s, e, interval.lc, interval.rc, result);
      } else {
        for (auto & h : cleaned)
          if (h.position >= s && h.position <= e)
            addMPointUnit(p0, p1, pIS, pIE, mpointUnit.id, 
              h.position, h.position, true, true, result);
      }
    }
  }

  inline void MRegions::calculateHits(int mregionUnit,
    Vec2 orig, Vec2 dir, double iS, double iL, std::set<Hit>& result)
  {
    double mregionTS = static_cast<double>(unit(mregionUnit).interval.s);
    double mregionTE = static_cast<double>(unit(mregionUnit).interval.e);

    if (mregionTS != mregionTE) {
      Vec3 orig3{ orig.x, orig.y, iS };
      Vec3 dir3{ dir.x, dir.y, iL};

      for (int fi = faceFirst(mregionUnit);
        fi < faceAfterLast(mregionUnit); fi++)
      {
        for (int ci = cycleFirst(fi); ci < cycleAfterLast(fi); ci++) {
          const int FIRST_EDGE = -1, NO_HIT = -2;
          int lastType = FIRST_EDGE;
          std::list<Hit> hits;

          for (int i = 0; i <= edgeCount(ci); i++) {
            int ei0 = edgeFirst(ci) + (i % edgeCount(ci));
            int ei1 = edgeFirst(ci) + ((i + 1) % edgeCount(ci));

            Edge & e0 = edge(ei0);
            Edge & e1 = edge(ei1);

            Vec3 e0s{ e0.s.x, e0.s.y, mregionTS };
            Vec3 e0e{ e0.e.x, e0.e.y, mregionTE };
            Vec3 e1s{ e1.s.x, e1.s.y, mregionTS };
            Vec3 e1e{ e1.e.x, e1.e.y, mregionTE };

            Hit hit;
            hit.edge = ei0;

            if (   ( !e0.s.almostEqual(e1.s) &&
                     calculateHit(orig3, dir3, e0s, e1s, e0e, hit) ) 
                || ( !e0.e.almostEqual(e1.e) &&
                     calculateHit(orig3, dir3, e1s, e1e, e0e, hit) ))
            {
              if (lastType != FIRST_EDGE && lastType != hit.type) 
                hits.push_back(hit);

              lastType = hit.type;
            } else {
              lastType = NO_HIT;
            }
          }

          for (auto & i : hits)
            result.insert(i);
        }
      }
    }
  }

  inline bool MRegions::calculateHit(const Vec3 & orig, const Vec3 & dir,
    const Vec3 & v0, const Vec3 & v1, const Vec3 & v2,
    Hit & result)
  {
    Vec3 v0v1 = v1 - v0;
    Vec3 v0v2 = v2 - v0;
    Vec3 pvec = dir.cross(v0v2);
    double det = v0v1.dot(pvec);

    result.type = det < 0.0 ? EXIT_POINT : ENTRY_POINT;

    if (std::abs(det) < SMALL_NUM) 
      return false;

    double invDet = 1 / det;

    Vec3 tvec = orig - v0;
    double u = tvec.dot(pvec) * invDet;
    if (u < 0 || u > 1) 
      return false;

    Vec3 qvec = tvec.cross(v0v1);
    double v = dir.dot(qvec) * invDet;
    if (v < 0 || u + v > 1) 
      return false;

    result.position = v0v2.dot(qvec) * invDet;

    return true;
  }

  inline bool MRegions::cleanHits(std::set<Hit>& hits, 
    std::list<Hit>& result, double start, double end, bool lc, bool rc)
  {
    std::list<Hit> combined, forwardDeduced, backwardDeduced;
    combineNearHits(hits, combined);
    deduceHitTypes(combined, true, forwardDeduced);
    bool typesKnown = deduceHitTypes(forwardDeduced, false, backwardDeduced);

    if (typesKnown) {
      auto i = backwardDeduced.begin();
      bool inside = backwardDeduced.front().type == EXIT_POINT;
      
      while (   i != backwardDeduced.end() 
             && (i->position < start || (!lc && i->position == start))) 
      {
        inside = i->type == ENTRY_POINT;
        i++;
      }

      if (inside)
        result.push_back(Hit{ start, lc ? LEFT_CLOSED : LEFT_OPEN, -1 });

      while (   i != backwardDeduced.end()
             && (i->position < end || (rc && i->position == end)))
      {
        inside = i->type == ENTRY_POINT;
        result.push_back(Hit{ i->position, 
                              inside ? LEFT_CLOSED : RIGHT_CLOSED, -1 });
        i++;
      }

      if (inside)
        result.push_back(Hit{ end, rc ? RIGHT_CLOSED : RIGHT_OPEN, -1 });

      return true;
    } else {
      for (auto & h : combined)
        if ((h.position > start || (h.position == start && lc)) &&
            (h.position < end   || (h.position == end   && rc)))
          result.push_back(h);

      return false;
    }
  }

  inline void MRegions::combineNearHits(std::set<Hit>& hits, 
    std::list<Hit>& result)
  {
    auto firstInGroup = hits.begin();
    while (firstInGroup != hits.end())
    {
      int entries = 0, exits = 0;
      auto last = firstInGroup, current = firstInGroup;

      while (current != hits.end() &&
        std::abs(current->position - last->position) < SMALL_NUM)
      {
        if (current->type == ENTRY_POINT)
          entries++;
        else
          exits++;

        last = current;
        current++;
      }

      Hit hit;
      hit.position = firstInGroup->position;
      hit.edge = -1;

      if (entries > exits)
        hit.type = ENTRY_POINT;
      else if (exits > entries)
        hit.type = EXIT_POINT;
      else
        hit.type = UNKNOWN;

      result.push_back(hit);
      firstInGroup = current;
    }
  }

  inline bool MRegions::deduceHitTypes(
    std::list<Hit> hits, bool forward, std::list<Hit> & result)
  {
    int last = UNKNOWN;

    auto deduce = [&last, &result](Hit & hit) {
      if (last == UNKNOWN || hit.type == ENTRY_POINT || 
                             hit.type == EXIT_POINT) 
      {
        result.push_back(hit);
        last = hit.type;
      } 
      else if (last == EXIT_POINT) 
      {
        hit.type = ENTRY_POINT;
        result.push_back(hit);
        hit.type = EXIT_POINT;
        result.push_back(hit);
        last = EXIT_POINT;
      }
    };

    if (forward)
      std::for_each(hits.begin(), hits.end(), deduce);
    else 
      std::for_each(hits.rbegin(), hits.rend(), deduce);

    return last != UNKNOWN;
  }

  inline int MRegions::relation(int mregionUnit, MPointsData::Unit mpointUnit, 
    int64_t time)
  {
    Interval & rI = unit(mregionUnit).interval;
    double rT, rIL = static_cast<double>(rI.e - rI.s);
    if (rIL != 0.0)
      rT = static_cast<double>(time - rI.s) / rIL;
    else
      rT = 0.0;

    Interval & pI = mpointUnit.interval;
    double pT, pIL = static_cast<double>(pI.e - pI.s);
    if (pIL != 0.0)
      pT = static_cast<double>(time - pI.s) / pIL;
    else
      pT = 0.0;

    Vec2 p{ (1.0 - pT) * mpointUnit.x0 + pT * mpointUnit.x1,
            (1.0 - pT) * mpointUnit.y0 + pT * mpointUnit.y1 };

    int hitCount = 0;

    for (int fi = faceFirst(mregionUnit); 
         fi < faceAfterLast(mregionUnit); fi++) 
    {
      for (int ci = cycleFirst(fi); ci < cycleAfterLast(fi); ci++) {
        for (int ei = edgeFirst(ci); ei < edgeAfterLast(ci); ei++) {
          int eiNext = ei < edgeAfterLast(ci) - 1 ? ei + 1 : edgeFirst(ci);
          Edge & e0 = edge(ei);
          Edge & e1 = edge(eiNext);

          Vec2 p0{ (1.0 - rT) * e0.s.x + rT * e0.e.x, 
                   (1.0 - rT) * e0.s.y + rT * e0.e.y };
          Vec2 p1{ (1.0 - rT) * e1.s.x + rT * e1.e.x, 
                   (1.0 - rT) * e1.s.y + rT * e1.e.y };
          Vec2 d = p1 - p0;

          if (d.x != 0.0) {
            double f = (p.x - p0.x) / d.x;

            if (f >= 0.0 && f < 1.0) {
              double y = p0.y + f * d.y;

              if (y == p.y)
                return BOUNDARY;
              else if (y > p.y)
                hitCount++;
            }
          } else if (p0.x == p.x) {
            if (   (p0.y <= p.y && p.y <= p1.y)
                || (p1.y <= p.y && p.y <= p0.y))
              return BOUNDARY;
          }
        }
      }
    }

    return hitCount % 2 == 1 ? INSIDE : OUTSIDE;
  }

  inline int64_t MRegions::bestTimeForRelation(int mregionUnit,
    double pIS, double pIL, std::list<Hit> hits)
  {
    Interval & rI = unit(mregionUnit).interval;

    std::list<int64_t> avoid;
    for (auto & h : hits) {
      int64_t t = static_cast<int64_t>(pIS + h.position * pIL);
      if (t > rI.s && t < rI.e)
        avoid.push_back(t);
    }
    avoid.push_back(rI.e);

    int64_t t;
    int64_t delta = -1;
    int64_t last = rI.s;

    for (auto & h : avoid) {
      if (h - last > delta) {
        t = (h + last) / 2;
        delta = h - last;
      }
      last = h;
    }

    return t;
  }

  inline void ColumnMovingAlgebra::MRegions::addMPointUnit(
    Vec2 p0, Vec2 p1, double pIS, double pIE, int id,
    double start, double end, bool lc, bool rc, 
    std::list<MPointsData::Unit>& result)
  {
    MPointsData::Unit u;
    u.x0 = (1.0 - start) * p0.x + start * p1.x;
    u.y0 = (1.0 - start) * p0.y + start * p1.y;
    u.x1 = (1.0 - end)   * p0.x + end   * p1.x;
    u.y1 = (1.0 - end)   * p0.y + end   * p1.y;

    double t0 = (1.0 - start) * pIS + start * pIE;
    double t1 = (1.0 - end)   * pIS + end   * pIE;

    u.interval.s = static_cast<int64_t>(t0);
    u.interval.e = static_cast<int64_t>(t1);
    u.interval.lc = lc;
    u.interval.rc = rc;

    u.id = id;

    if (u.interval.s < u.interval.e || (lc && rc)) {
      if (result.size() > 0) {
        MPointsData::Unit & last = result.back();
        Interval & lastI = last.interval;
        Interval & uI = u.interval;

        if (lastI.e == uI.s && (lastI.rc || uI.lc) && last.id == u.id)
        {
          lastI.e = uI.e;
          lastI.rc = uI.rc;
          last.x1 = u.x1;
          last.y1 = u.y1;
          return;
        }
      }

      result.push_back(u);
    }
  }
}
