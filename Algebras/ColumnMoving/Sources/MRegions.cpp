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

1 MRegions.cpp

*/

#include "MRegions.h"

using namespace std;

extern NestedList* nl;

namespace ColumnMovingAlgebra
{
/*
1.1 Implementation of Constants for ~MRegions~

~SMALLNUM~ represents a small number as cut off value in the intersection
calculations

*/
  const double MRegions::SMALL_NUM = 1e-8;

/*
1.1 Implementation of Virtual Functions for ~MRegions~

the following functions are required by the crel algebra for all attribute 
arrays.

~Filter~ returns a duplicate of this attribut array with the speficied filter.
 
*/
  CRelAlgebra::AttrArray* MRegions::Filter(
    CRelAlgebra::SharedArray<const uint64_t> filter) const
  {
    return new MRegions(*this, filter);
  }

/*
~GetCount~ returns the number of entries in the attribut array.

*/
  uint64_t MRegions::GetCount() const
  {
    return m_MRegions->size();
  }

/*
~GetSize~ returns the amount of space needed to save this attribut array
to persistant storage.

*/
  uint64_t MRegions::GetSize() const
  {
    return  m_Edges->savedSize() +
            m_Cycles->savedSize() +
            m_Faces->savedSize() +
            m_Units->savedSize() +
            m_MRegions->savedSize() +
            m_DefTimes->savedSize();
  }

/*
~Save~ saves this attribut array
to persistant storage.

*/
  void MRegions::Save(CRelAlgebra::Writer &target, bool includeHeader) const
  {
    m_Edges->save(target);
    m_Cycles->save(target);
    m_Faces->save(target);
    m_Units->save(target);
    m_MRegions->save(target);
    m_DefTimes->save(target);
  }

/*
~Append~ adds the moving point at index ~row~ of the attribut array ~array~

*/
  void MRegions::Append(const CRelAlgebra::AttrArray & array, uint64_t row)
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

/*
~Append~ adds the row orientied MPoint ~value~

*/
  void MRegions::Append(Attribute & value)
  {
    auto & m = static_cast<temporalalgebra::MRegion&>(value);
    auto segments = static_cast<DbArray<temporalalgebra::MSegmentData>*>
      (m.GetFLOB(1));

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

    checkMRegion(m_MRegions->size() - 1);
  }

/*
~Remove~ removes the last added moving point

*/
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

/*
~Clear~ removes all moving points

*/
  void MRegions::Clear()
  {
    m_Edges->clear();
    m_Cycles->clear();
    m_Faces->clear();
    m_Units->clear();
    m_MRegions->clear();

    m_DefTimes->clear();
  }

/*
~IsDefined~ returns true, iff the moving point with index ~row~ has any units

*/
  bool MRegions::IsDefined(uint64_t row) const
  {
    return unitCount(row) > 0;
  }

/*
~Compare~ compares the moving point at index ~rowA~ with the moving point
at index ~rowB~ in ~arrayB~

*/
  int MRegions::Compare(uint64_t rowA, const CRelAlgebra::AttrArray &arrayB,
    uint64_t rowB) const
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

/*
~Compare~ compares the moving point at index ~rowA~ with the row oriented
attribute ~value~

*/
  int MRegions::Compare(uint64_t row, Attribute &value) const
  {
    MRegions mrs;
    mrs.Append(value);
    return Compare(row, mrs, 0);
  }

/*
~GetHash~ returns a hash value for the moving point at index ~row~

*/
  uint64_t MRegions::GetHash(uint64_t row) const
  {
    if (m_Units->size() == 0)
      return 0;

    return (uint64_t)(unit(0).interval.s ^ unit(0).interval.e);
  }


/*
~GetAttribute~ converts the moving point 
in ~row~ to an MPoint as defined in the temporal algebra for row oriented
relations and returns it.

*/
  Attribute * MRegions::GetAttribute(uint64_t row, bool clone) const
  {
    temporalalgebra::MRegion* mr = new temporalalgebra::MRegion(0);

    for (int ui = unitFirst(row); ui < unitAfterLast(row); ui++) {
      std::vector<temporalalgebra::MSegmentData> linelist;    

      for (int fi = faceFirst(ui), fii = 0; 
           fi < faceAfterLast(ui); fi++, fii++) 
      {
        for (int ci = cycleFirst(fi), cii = 0; 
             ci < cycleAfterLast(fi); ci++, cii++) 
        {          
          for (int i = 0; i < edgeCount(ci); i++) {
            int ei0 = edgeFirst(ci) + (i % edgeCount(ci));
            int ei1 = edgeFirst(ci) + ((i + 1) % edgeCount(ci));

            Edge & e0 = edge(ei0);
            Edge & e1 = edge(ei1);
            
            bool insideAbove;
            
            if (AlmostEqual(e0.s.x, e1.s.x) && AlmostEqual(e0.s.y, e1.s.y)) {
              bool eqx = AlmostEqual(e0.e.x, e1.e.x);
              insideAbove = (!eqx && (e0.e.x < e1.e.x) ) ||
                (eqx && !AlmostEqual(e0.e.y, e1.e.y) && (e0.e.y < e1.e.y));
            } else {
              bool eqx = AlmostEqual(e0.s.x, e1.s.x);
              insideAbove = (!eqx && (e0.s.x < e1.s.x) ) ||
                (eqx && !AlmostEqual(e0.s.y, e1.s.y) && (e0.s.y < e1.s.y));
            }
            insideAbove = insideAbove == (ci == cycleFirst(fi));
                 
            linelist.emplace_back(fii, cii, i, insideAbove, 
              e0.s.x, e0.s.y, e1.s.x, e1.s.y,
              e0.e.x, e0.e.y, e1.e.x, e1.e.y);
          }
        }
      }

      temporalalgebra::URegion ur(linelist, unit(ui).interval.convert(), true);
      mr->AddURegion(ur);
    }

    return mr;
  }
  
}
