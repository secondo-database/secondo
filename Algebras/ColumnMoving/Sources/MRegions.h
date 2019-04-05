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

1 MRegions.h

*/

#pragma once

#include <memory>
#include "MObjects.h"
#include "MPoints.h"
#include "IRegions.h"
#include "Algebras/Spatial/RegionTools.h"

namespace ColumnMovingAlgebra
{
/*
1.1 Declaration of the class ~MRegions~

~MRegions~ represents a moving regions attribute array.

The logic structure of a region that we use consists of multiple faces.
Each face consists of one or more cycles which are two-dimensional
polygons. The first cycle of a face is 
the outer cycle and all other cycles of the face
are hole cycles. To construct the area of a face, we subtract the hole
cycles from the outer cycle. The area of a region is the union of the 
area of its faces.

A moving region unit has the same structure as a region, but the cycles are
now a list of edges. Each edge has a two-dimensional
start and end point. Furthermore the 
moving region unit has a time interval that determines when the unit is 
defined.

During the time interval we can determine the area of a moving region unit for 
a given time point by linear interpolation between
start and end point for all of its edges. The result of
the interpolation is a two-dimensional point for each edge. If we connect 
these points in the order of their corresponding edges we get a polygon
for each cycle and therefore the region corresponding to the time point.

A moving region is a collection of moving region units with disjoint
time intervals.

*/
  class MRegions : public MObjects
  {
  public:
/*
1.1.1 Nested Structs

~Vec2~ represents a 2D-dimensional 

*/
    struct Vec2 {
      double x, y;
      bool almostEqual(const Vec2 & b) const;
      Vec2 operator - (const Vec2 & b) const;
      Vec2 operator + (const Vec2 & b) const;
    };
/*
~Edge~ represents an edge of a moving region.

*/
    struct Edge { Vec2 s, e; };

/*
1.1.1 Constructors 

The following constructor signatures are required by the crel algebra for all 
attribute arrays.

*/
    MRegions();
    MRegions(CRelAlgebra::Reader& source);
    MRegions(CRelAlgebra::Reader& source, uint64_t rowsCount);
    MRegions(const MRegions &array, 
             const CRelAlgebra::SharedArray<const uint64_t> &filter);
/*
1.1.2 Destructor

*/
    virtual ~MRegions() { }

/*
1.1.3 CRel Algebra Interface

the following functions are required by the crel algebra for all attribute 
arrays.

~Filter~ returns a duplicate of this attribut array with the speficied filter.

 
*/
    virtual AttrArray* Filter(
      CRelAlgebra::SharedArray<const uint64_t> filter) const;

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
~GetAttribute~ converts the moving point 
in ~row~ to an MPoint as defined in the temporal algebra for row oriented
relations and returns it.

*/
    virtual Attribute *GetAttribute(uint64_t row, bool clone = true) const;

/*
~Save~ saves this attribut array
to persistant storage.

*/
    virtual void Save(CRelAlgebra::Writer &target, 
                      bool includeHeader = true) const;

/*
~Append~ adds the moving point at index ~row~ of the attribut array ~array~

*/
    virtual void Append(const CRelAlgebra::AttrArray & array, uint64_t row);
/*
or adds the row orientied MPoint ~value~

*/
    virtual void Append(Attribute & value);
/*
~Remove~ removes the last added moving point

*/
    virtual void Remove();
/*
~Clear~ removes all moving points

*/
    virtual void Clear();

/*
~IsDefined~ returns true, iff the moving point with index ~row~ has any units

*/
    virtual bool IsDefined(uint64_t row) const;

/*
~Compare~ compares the moving point at index ~rowA~ with the moving point
at index ~rowB~ in ~arrayB~

*/
    virtual int  Compare(uint64_t rowA, const AttrArray& arrayB, 
      uint64_t rowB) const;
/*
~Compare~ compares the moving point at index ~rowA~ with the row oriented
attribute ~value~

*/
    virtual int  Compare(uint64_t row, Attribute &value) const;

/*
~GetHash~ returns a hash value for the moving point at index ~row~

*/
    virtual uint64_t GetHash(uint64_t row) const;

/*
1.1.2 Operators

The following functions implement the operators supported by moving regions
attribute array. 

~atInstant~ is a timeslice operator and computes an intime for all 
moving regions in the attribute array and adds them to ~result~

*/
    void atInstant(Instant instant, IRegions & result);
/*
~atPeriods~ restricts the moving regions to a given set of time 
intervals and adds the resulting units to ~result~.

*/
    void atPeriods(temporalalgebra::Periods periods, MRegions & result);
/*
~intersection~ restricts the moving point ~mpoints~ to the time intervals
when it is in the moving region 

*/
    void intersection(MPoints mpoints, MPoints & result);

/*
1.1.1 Data Update Functions

~addMRegion~ adds a new moving region to the attribute array

*/
    void addMRegion();
/*
~addUnit~ adds a new unit to the last added moving region

*/
    void addUnit(Interval interval);
/*
~addFace~ adds a new face to the last added unit

*/
    void addFace();
/*
~addCycle~ adds a new cycle to the last added face

*/
    void addCycle();
/*
~addEdge~ adds a new edge to the last added cycle

*/
    void addEdge(Edge edge);

/*
~addConstMRegion~ adds a new moving region that has one constant
unit has the value ~region~ during the time interval ~interval~

*/
    void addConstMRegion(Region & region, Interval interval);

/*
~checkMRegion~ makes sure that all outer cycles of the moving region with
index ~index~ have mathematical positive rotation direction and all
hole cycles have mathematical negative rotation direction 

*/
    void checkMRegion(int index);
    
  private:
/*
1.1.1 Constants

~SMALLNUM~ represents a small number as cut off value in the intersection
calculations

*/
    static const double SMALL_NUM;

/*
the following constants determine, whether a moving point enters or exits
a moving region at a certain time

*/
    static const int ENTRY_POINT = 0, EXIT_POINT = 1, UNKNOWN = 2;
/*
the following constants are used to determine, whether a time point is
the beginning or end of a intersection unit and whether the corresponding
interval boundary is open or closed.

*/
    static const int LEFT_CLOSED = 3, LEFT_OPEN = 4, 
      RIGHT_CLOSED = 5, RIGHT_OPEN = 6;
/*
the following constants determine, whether a moving point is inside, on
the boundary or outside of a moving region at a certain time point

*/
    static const int INSIDE = 0, BOUNDARY = 1, OUTSIDE = 2;

/*
1.1.1 Data Representation

The following arrays represent the logic structure of moving regions as
explained above.

*/
    struct Cycle { int firstEdge; };
    struct Face { int firstCycle; };
    struct Unit { Interval interval; int firstFace; Vec2 min, max; };
    struct MRegion { int firstUnit; };

    std::shared_ptr<Array<Edge>> m_Edges;
    std::shared_ptr<Array<Cycle>> m_Cycles;
    std::shared_ptr<Array<Face>> m_Faces;
    std::shared_ptr<Array<Unit>> m_Units;
    std::shared_ptr<Array<MRegion>> m_MRegions;

/*
1.1.1 Data Access Convenience Functions

The following functions are used to access the data of the arrays. They
are for convenience and readability of the source code.

*/
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

/*
1.1.1 Intersection 

The following data structeres, constants and functions are used to 
calculate the intersection between a moving point unit and a moving region
unit.

The algorithm has checks both units for overlap in 
spatial and the time dimension. Then intersection points between the
moving point unit and the boundary of the moving region (hit points) 
unit are calculated
and categorized in enter and exit points. Finally we build new moving
point units between the enter and the following exit points.

As the boundaries of cycles are allowed to touch, there might be multiple
hit points with identical coordinates. In these cases we cannot directly 
build the result units from the hit points. So a include a 
intermediate step in which we merge hit points with identical coordinates.

If the moving point unit is completely inside or outside the moving region
unit, there will be no hit points. In this case we have to determine the 
wether the moving point unit is inside or outside
of the moving point unit at any time point to deduce whether there is a
intersection.

~Vec3~ represents a simple three-dimensional vector.

*/
    struct Vec3 {
      double x, y, t;
      Vec3 operator-(const Vec3 & b) const;
      double dot(const Vec3 & b) const;
      Vec3 cross(const Vec3 & b) const;
    };

/*
~Hit~ represents a hit point on which a moving point hits a moving region.
~position~ determines the relativ position on the time interval
of the moving point unit, so 0 is at the beginning and 1 is at the end
of the moving point unit. ~type~ determines
if this is an entry or exit point. 

*/
    struct Hit { 
      double position; 
      int type; 
      int edge;
      bool operator < (const Hit & b) const;
    };

/*
~MPointUnit~ represents a unit of a moving point as an intermediate
result of the intersection calculation.

*/
    struct MPointUnit {
      Interval interval;
      Vec2 s, e;
    };

/*
~cycleIsClockwise~ determines, wether the cycle with index ~cycle~ 
is in mathematical positive rotation direction

*/
    bool cycleIsClockwise(int cycle);
/*
~cycleReverse~ reverses the order of edges of the cycle with index ~cycle~ 

*/
    void cycleReverse(int cycle);

/*
~intersection~ calculates the intersection of a moving point unit and a
moving region unit and adds the resulting new units to ~result~

*/
    void intersection(int mregionUnit, MPointsData::Unit mpointUnit,
      std::list<MPointsData::Unit> & result);
/*
~hasSpatialOverlap~ checks wether a moving region unit has a overlap in
the spatial dimensions with the moving point unit

*/
    bool hasSpatialOverlap(int mregionUnit, MPointsData::Unit mpointUnit);
/*
~calculateHits~ checks for hit points between a moving region unit and
a moving point unit. the moving point unit is defined by a spatial origin
~orig~, spatial direction ~dir~ and the start and end of its time interval 
~iS~ and ~iE~. 

*/
    void calculateHits(int mregionUnit, Vec2 orig, Vec2 dir, double iS, 
      double iL, std::set<Hit> & result);
/*
~calculateHit~ helper function for a three-dimensional hit test between
the ray defined by ~orig~ and ~dir~ and a triangle defined by 
the three points ~v0~, ~v1~ and ~v2~. If a intersection exits it returns true
and sets ~result~ 

*/
    bool calculateHit(const Vec3 & orig, const Vec3 & dir,
      const Vec3 & v0, const Vec3 & v1, const Vec3 & v2,
      Hit & result);
/*
~combineNearHits~ finds hit points that are close together
and merges them

*/
    void combineNearHits(std::set<Hit> & hits, std::list<Hit> & result);
/*
~cleanHits~ calls combineNearHits to merge hit points that are close
together and then calls deduceHitTypes to try to determine whether
the fusion points have the type entry or exit point

*/
    bool cleanHits(std::set<Hit> & hits, std::list<Hit> & result,
      double start, double end, bool lc, bool rc);
/*
~deduceHitTypes~ trys to determine, whether
the fusion points have the type entry or exit point

*/
    bool deduceHitTypes(std::list<Hit> hits, bool forward, 
      std::list<Hit> & result);
/*
In cases, when the point region unit has no intersection with the
moving region unit (or only point intersections), we will find no
(or no unambiguous) hit points. The moving point unit might be completely 
inside the moving region unit or might be outside.

~bestTimeForRelation~ finds a time point during the mregionUnit definition
interval that is as far away as possible from the definition interval
boundaries (and from all hit ambiguous hit points). 
This time point is optimal to avoid problems during
a test that determines wether the moving point unit is inside or 
outside the moving region unit. 

*/
    int64_t bestTimeForRelation(int mregionUnit, double pIS, double pIL,
      std::list<Hit> hits);
/*
~relation~ determines, whether a moving point unit
is inside or
outside the moving region unit at the instant ~time~.

*/
    int relation(int mregionUnit, MPointsData::Unit mpointUnit, int64_t time);
/*
~addMPointUnit~ constructs the result units of the intersection operator

*/
    void addMPointUnit(Vec2 p0, Vec2 p1, double pIS, double pIE, int id,
      double start, double end, bool lc, bool rc, 
      std::list<MPointsData::Unit> & result);
  };

/*
1.1 Implementation of the ~MRegions~

1.1.1 Constructors 

The following constructor signatures are required by the crel algebra for all 
attribute arrays.

*/
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

  inline MRegions::MRegions(CRelAlgebra::Reader& source, uint64_t rowsCount)
  {
    m_Edges = std::make_shared<Array<Edge>>(source);
    m_Cycles = std::make_shared<Array<Cycle>>(source);
    m_Faces = std::make_shared<Array<Face>>(source);
    m_Units = std::make_shared<Array<Unit>>(source);
    m_MRegions = std::make_shared<Array<MRegion>>(source);
    m_DefTimes = std::make_shared<DefTimes>(source);
  }

  inline MRegions::MRegions(const MRegions &array, 
    const CRelAlgebra::SharedArray<const uint64_t> &filter) :
    MObjects(array, filter),
    m_Edges(array.m_Edges),
    m_Cycles(array.m_Cycles),
    m_Faces(array.m_Faces),
    m_Units(array.m_Units),
    m_MRegions(array.m_MRegions)
  {
  }
  
/*
1.1.2 Operators

The following functions implement the operators supported by moving regions
attribute array. 

~atInstant~ is a timeslice operator and computes an intime for all 
moving regions in the attribute array and adds them to ~result~

*/
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

/*
~atPeriods~ restricts the moving regions to a given set of time 
intervals and adds the resulting units to ~result~.

*/
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

/*
~intersection~ restricts the moving point ~mpoints~ to the time intervals
when it is in the moving region 

*/
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
  
/*
1.1.1 Implementation of the Nested Class ~Vec2~

*/

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

/*
1.1.1 Data Update Functions

~addMRegion~ adds a new moving region to the attribute array

*/
  inline void MRegions::addMRegion()
  {
    m_MRegions->emplace_back(MRegion{ static_cast<int>(m_Units->size()) });
    m_DefTimes->addRow();
  }

/*
~addUnit~ adds a new unit to the last added moving region

*/
  inline void MRegions::addUnit(Interval interval)
  {
    m_Units->emplace_back(Unit{ interval, 
                                static_cast<int>(m_Faces->size()) });
    m_DefTimes->addInterval(interval);
  }
  
/*
~addFace~ adds a new face to the last added unit

*/
  inline void MRegions::addFace()
  {
    m_Faces->emplace_back(Face{ static_cast<int>(m_Cycles->size()) });
  }
  
/*
~addCycle~ adds a new cycle to the last added face

*/
  inline void MRegions::addCycle()
  {
    m_Cycles->emplace_back(Cycle{ static_cast<int>(m_Edges->size()) });
  }
  
/*
~addEdge~ adds a new edge to the last added cycle

*/
  inline void MRegions::addEdge(Edge edge)
  {
    m_Edges->push_back(edge);
    
    Vec2 min, max;
    
    if (edge.s.x < edge.e.x) {
      min.x = edge.s.x;
      max.x = edge.e.x;
    } else {
      min.x = edge.e.x;
      max.x = edge.s.x;
    } 
    
    if (edge.s.y < edge.e.y) {
      min.y = edge.s.y;
      max.y = edge.e.y;
    } else {
      min.y = edge.e.y;
      max.y = edge.s.y;
    } 
    
    Unit &u = m_Units->back();
    Face &f = m_Faces->back();
    Cycle &c = m_Cycles->back();
    
    bool firstEdgeOfUnit = u.firstFace  == (int) m_Faces->size()  - 1 &&
                           f.firstCycle == (int) m_Cycles->size() - 1 &&
                           c.firstEdge  == (int) m_Edges->size()  - 1;         
    
    if (firstEdgeOfUnit) {
      u.min.x = min.x;
      u.min.y = min.y;
      u.max.x = max.x;
      u.max.y = max.y;
    } else {
      if (min.x < u.min.x)
        u.min.x = min.x;
        
      if (min.y < u.min.y)
        u.min.y = min.y;
        
      if (max.x > u.max.x)
        u.max.x = max.x;
        
      if (max.y > u.max.y)
        u.max.y = max.y;
    }
  }
  
/*
~addConstMRegion~ adds a new moving region that has one constant
unit has the value ~region~ during the time interval ~interval~

*/
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
          checkr(false, "Wrong data format: discontiguous segments");
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
              checkr(false, "Wrong data format: discontiguous segments");
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
              checkr(false, "Wrong data format: discontiguous segments");
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
            checkr(false, "Wrong data format: discontiguous segments");
          }

          addEdge(Edge{{outputP.GetX(), outputP.GetY()},
                                          {outputP.GetX(), outputP.GetY()}});
        }
      }
    }
    
    checkMRegion(0);
    RCopy->DeleteIfAllowed();
  }

/*
~checkMRegion~ makes sure that all outer cycles of the moving region with
index ~index~ have mathematical positive rotation direction and all
hole cycles have mathematical negative rotation direction 

*/
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
/*
1.1.1 Data Access Convenience Functions

The following functions are used to access the data of the arrays. They
are for convenience and readability of the source code.

*/
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

/*
1.1.1 Implementation of the Nested Class ~Vec3~

*/
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

/*
1.1.1 Intersection 

~intersection~ calculates the intersection of a moving point unit and a
moving region unit.

*/
  inline void MRegions::intersection(int mregionUnit, 
    MPointsData::Unit mpointUnit, std::list<MPointsData::Unit> & result)
  {
    Interval & rI = unit(mregionUnit).interval;
    Interval & pI = mpointUnit.interval;
    
    //we check for overlap first

    if (!rI.intersects(pI))
      return;
      
    if (!hasSpatialOverlap(mregionUnit, mpointUnit))
      return;

    Interval interval = rI.intersection(pI);

    Vec2 p0 = Vec2{ mpointUnit.x0, mpointUnit.y0 };
    Vec2 p1 = Vec2{ mpointUnit.x1, mpointUnit.y1 };
    double pIS = static_cast<double>(mpointUnit.interval.s);
    double pIE = static_cast<double>(mpointUnit.interval.e);
    double pIL = pIE - pIS, s, e;
    
    //if the moving point unit is short, we will make it longer
    //because we want to identify hit points by the relative
    //position on the moving point unit

    if (pIL <= SMALL_NUM) {
      s = e = 0.0;
      pIL = 1.0;
    } else {
      s = (interval.s - pI.s) / pIL;
      e = (interval.e - pI.s) / pIL;
    }
    
    //now we can calculate the hit points

    std::set<Hit> hits;
    calculateHits(mregionUnit, p0, p1 - p0, pIS, pIL, hits);
    std::list<Hit> cleaned;
    bool hitTypesKnown = cleanHits(hits, cleaned, s, e, 
                                   interval.lc, interval.rc);

    if (hitTypesKnown) {
      
      //in most cases we can deduce the result units directly from the
      //hit points
      
      double p = 0.0;
      bool lc=true;

      for (auto & h : cleaned)
        if (h.type == LEFT_CLOSED || h.type == LEFT_OPEN) {
          p = h.position;
          lc = h.type == LEFT_CLOSED;
        } else {
          addMPointUnit(p0, p1, pIS, pIE, mpointUnit.id,
            p, h.position, lc, h.type == RIGHT_CLOSED, result);
        }
    } else {
      
      //if we did not find any clear entry or exit points, then we
      //will check whether the moving point unit is inside or
      //outside at a suitable poing
      
      int64_t t = bestTimeForRelation(mregionUnit, pIS, pIE, cleaned);

      if (relation(mregionUnit, mpointUnit, t) != OUTSIDE) {
      
        //the moving point unit is completely inside
        
        addMPointUnit(p0, p1, pIS, pIE, mpointUnit.id, 
          s, e, interval.lc, interval.rc, result);
      } else {
      
        //the moving point unit is outside but might have point intersections
        //with the moving region unit
        
        for (auto & h : cleaned)
          if (h.position >= s && h.position <= e)
            addMPointUnit(p0, p1, pIS, pIE, mpointUnit.id, 
              h.position, h.position, true, true, result);
      }
    }
  }
  
/*
~hasSpatialOverlap~ checks wether a moving region unit has a overlap in
the spatial dimensions with the moving point unit

*/
  inline bool MRegions::hasSpatialOverlap(int mregionUnit, 
    MPointsData::Unit mpointUnit)
  {
    Vec2 min, max;
    
    if (mpointUnit.x0 < mpointUnit.x1) {
      min.x = mpointUnit.x0;
      max.x = mpointUnit.x1;
    } else {
      min.x = mpointUnit.x1;
      max.x = mpointUnit.x0;
    } 
    
    if (mpointUnit.y0 < mpointUnit.y1) {
      min.y = mpointUnit.y0;
      max.y = mpointUnit.y1;
    } else {
      min.y = mpointUnit.y1;
      max.y = mpointUnit.y0;
    } 

    Unit &u = unit(mregionUnit);
    
    if (u.min.x > max.x)
      return false;
    
    if (u.max.x < min.x)
      return false;
    
    if (u.min.y > max.y)
      return false;
    
    if (u.max.y < min.y)
      return false;
      
    return true;
  }

/*
~calculateHits~ checks for hit points between a moving region unit and
a moving point unit. the moving point unit is defined by a spatial origin
~orig~, spatial direction ~dir~ and start and end point of its time interval 
~iS~ and ~iE~. 

*/
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
              //we have to avoid to add a hit point twice if the
              //moving point units intersects one of the edges
              
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

/*
~calculateHit~ helper function for a three-dimensional hit test between
a ray and a triangle

*/
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

/*
~cleanHits~ calls combineNearHits to merge hit points that are close
together and then calls deduceHitTypes to try to determine whether
the fusion points have the type entry or exit point

*/
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

/*
~combineNearHits~ finds hit points that are close together
and merges them

*/
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

/*
~deduceHitTypes~ trys to determine, whether
the fusion points have the type entry or exit point

*/
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

/*
~relation~ determines, whether a moving point unit
is inside or
outside the moving region unit at the instant ~time~.
this is done in 2 dimension by simply calculating the
number of intersection with boundaries of the cycles

*/
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

/*
~bestTimeForRelation~ finds a time point during the mregionUnit definition
interval that is as far away as possible from the definition interval
boundaries (and from all hit ambiguous hit points). 
This time point is optimal to avoid problems during
a test that determines wether the moving point unit is inside or 
outside the moving region unit. 

*/
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

    int64_t t=0;
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

/*
~addMPointUnit~ constructs the result units of the intersection operator

*/
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
