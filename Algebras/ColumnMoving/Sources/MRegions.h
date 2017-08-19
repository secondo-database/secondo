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
#include "MPoints.h"
#include "IRegions.h"

namespace ColumnMovingAlgebra
{
  class MRegions : public CRelAlgebra::AttrArray
  {
  public:
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
    void atInstant(Instant instant, IRegions & result);
    void atPeriods(temporalalgebra::Periods periods, MRegions & result);
    void intersection(MPoints mpoints, MPoints & result);
    
  private:
    struct Point { double x, y; };
    struct Edge { Point s, e; };
    struct Cycle { int firstEdge; };
    struct Face { int firstCycle; };
    struct Unit { Interval interval; int firstFace; };
    struct MRegion { int firstUnit; };

    shared_ptr<Array<Edge>> m_Edges;
    shared_ptr<Array<Cycle>> m_Cycles;
    shared_ptr<Array<Face>> m_Faces;
    shared_ptr<Array<Unit>> m_Units;
    shared_ptr<Array<MRegion>> m_MRegions;
    shared_ptr<DefTimes> m_DefTimes;

    void addMRegion();
    void addUnit(Interval interval);
    void addFace();
    void addCycle();
    void addEdge(Edge edge);
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
  };




  inline MRegions::MRegions() :
    m_Edges(make_shared<Array<Edge>>()),
    m_Cycles(make_shared<Array<Cycle>>()),
    m_Faces(make_shared<Array<Face>>()),
    m_Units(make_shared<Array<Unit>>()),
    m_MRegions(make_shared<Array<MRegion>>()),
    m_DefTimes(make_shared<DefTimes>())
  {
  }

  inline MRegions::MRegions(CRelAlgebra::Reader& source)
  {
    m_Edges = make_shared<Array<Edge>>(source);
    m_Cycles = make_shared<Array<Cycle>>(source);
    m_Faces = make_shared<Array<Face>>(source);
    m_Units = make_shared<Array<Unit>>(source);
    m_MRegions = make_shared<Array<MRegion>>(source);
    m_DefTimes = make_shared<DefTimes>(source);
  }

  inline MRegions::MRegions(CRelAlgebra::Reader& source, size_t rowsCount)
  {
    m_Edges = make_shared<Array<Edge>>(source);
    m_Cycles = make_shared<Array<Cycle>>(source);
    m_Faces = make_shared<Array<Face>>(source);
    m_Units = make_shared<Array<Unit>>(source);
    m_MRegions = make_shared<Array<MRegion>>(source);
    m_DefTimes = make_shared<DefTimes>(source);
  }

  inline MRegions::MRegions(const MRegions &array, 
    const CRelAlgebra::SharedArray<const size_t> &filter) :
    AttrArray(filter),
    m_Edges(array.m_Edges),
    m_Cycles(array.m_Cycles),
    m_Faces(array.m_Faces),
    m_Units(array.m_Units),
    m_MRegions(array.m_MRegions),
    m_DefTimes(array.m_DefTimes)
  {
  }

  inline void MRegions::addMRegion()
  {
    m_MRegions->emplace_back(MRegion{ static_cast<int>(m_Units->size()) });
    m_DefTimes->addRow();
  }
  
  inline void MRegions::addUnit(Interval interval)
  {
    m_Units->emplace_back(Unit{ interval, static_cast<int>(m_Faces->size()) });
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
}
