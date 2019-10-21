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
MERCHANTABILITY or FITNESS FOR Line PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SECONDO; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
----

*/

#pragma once

#include <algorithm>
#include "Attribute.h"
#include <cstdint>
#include <cstring>
#include "AlmostEqual.h"
#include "Algebras/Geoid/GeoDist.h"
#include "Algebras/Spatial/HalfSegment.h"
#include <limits>
#include <cmath>
#include "Algebras/Spatial/Point.h"
#include "SecondoException.h"
#include "SimpleAttrArray.h"
#include "Algebras/Spatial/SpatialAlgebra.h"

namespace CRelAlgebra
{
  class LineEntry
  {
  public:
    typedef Line AttributeType;

    static const bool isPrecise = true;

    static const int dimension = 2;

    static uint64_t GetSize(const Line &value)
    {
      if (value.IsDefined())
      {
        return sizeof(int64_t) + sizeof(Rectangle<2>) +
               (value.Size() * sizeof(SimpleHalfSegment));
      }

      return 0;
    }

    static void Write(SimpleVSAttrArrayEntry target, const Line &value)
    {
      if (value.IsDefined())
      {
        LineEntry entry = LineEntry(target);

        new (entry.GetCountP()) int64_t(value.Size());

        value.SelectFirst();

        HalfSegment sourceSegment;

        for (SimpleHalfSegment &targetSegment : entry)
        {
          value.GetHs(sourceSegment);

          const SimpleHalfSegment segment = SimpleHalfSegment(sourceSegment);

          new (&targetSegment) SimpleHalfSegment(segment);

          value.SelectNext();
        }

        new (entry.GetBoundingBoxP()) Rectangle<2>(value.BoundingBox());
      }
    }

    LineEntry()
    {
    }

    LineEntry(const SimpleVSAttrArrayEntry &value) :
      data(value.data),
      size(value.size)
    {
    }

    bool IsDefined() const
    {
      return size != 0;
    }

    int Compare(const LineEntry &other) const
    {
      if (size > other.size)
      {
        return 1;
      }

      if (size < other.size)
      {
        return -1;
      }

      if (size == 0)
      {
        return 0;
      }

      const int64_t count = *GetCountP();

      const SimpleHalfSegment *segmentsA = begin(),
        *segmentsB = other.begin();

      int cmp = 0;

      for (int64_t i = 0; cmp == 0 && i < count; ++i)
      {
        cmp = segmentsA[i].Compare(segmentsB[i]);
      }

      return cmp;
    }

    int Compare(const Line &value) const
    {
      if (size == 0)
      {
        return value.IsDefined() ? -1 : 0;
      }

      if (!value.IsDefined())
      {
        return 1;
      }

      const int64_t countA = *GetCountP(),
        countB = value.Size();

      if (countA > countB)
      {
        return 1;
      }

      if (countA < countB)
      {
        return -1;
      }

      int cmp = 0;

      if (countA > 0)
      {
        HalfSegment segmentB;

        value.SelectFirst();

        for (const SimpleHalfSegment& segmentA : *this)
        {
          value.GetHs(segmentB);

          cmp = segmentA.Compare(SimpleHalfSegment(segmentB));

          if (cmp != 0)
          {
            break;
          }
        }
      }

      return cmp;
    }
    
    template<class Z>
    int CompareAlmost(const Z& z) const{
      return Compare(z);
    }

    bool Equals(const LineEntry &other) const
    {
      return Compare(other) == 0;
    }

    bool Equals(const Line &value) const
    {
      return Compare(value) == 0;
    }

    template<class Z>
    bool EqualsAlmost(const Z& z) const{
      return Equals(z);
    }


    uint64_t GetHash() const
    {
      uint64_t h = 0;

      if (size != 0)
      {
        char i = 0;

        for (const SimpleHalfSegment& segment : *this)
        {
          h += (uint64_t)((5 * segment.lp.x + segment.lp.y) +
                        (5 * segment.rp.x + segment.rp.y));

          if (++i == 5)
          {
            break;
          }
        }
      }

      return h;
    }

    Line *GetAttribute(bool clone = true) const
    {
      if (size != 0)
      {
        const int64_t count = *GetCountP();

        Line *line = new Line(count);

        if (count > 0)
        {
          /*const Rectangle<2> &bbox = *GetBoundingBoxP();

          *line += HalfSegment(true, Point(true, bbox.MinD(0), bbox.MinD(1)),
                                     Point(true, bbox.MaxD(0), bbox.MaxD(1)));

          int i = 0;*/

          for (const SimpleHalfSegment& segment : *this)
          {
            //This causes significant overhead but there is currently no other
            //way to ensure a correct bounding box
            *line += segment.ToHalfSegment();

            //line->Put(i++, segment.ToHalfSegment());
          }
        }

        return line;
      }

      return new Line(false);
    }

    virtual Rectangle<2> GetBoundingBox(const Geoid *geoid = nullptr) const
    {
      if (size != 0)
      {
        if (geoid == nullptr) // euclidean
        {
          const double left = GetBoundingBoxP()->MinD(0),
            top = GetBoundingBoxP()->MaxD(1),
            right = GetBoundingBoxP()->MaxD(0),
            bottom = GetBoundingBoxP()->MinD(1);
          double minMax[] = {left, right, bottom, top};
          return Rectangle<2>(true, minMax);

          //return *GetBoundingBoxP();
        }

        if (geoid->IsDefined()) // spherical
        {
          Rectangle<2> bbox = Rectangle<2>(false);

          for (const SimpleHalfSegment& segment : *this)
          {
            if (segment.ldp) // ignore inverse HalfSegments
            {
              if (!bbox.IsDefined())
              {
                bbox = segment.GetBoundingBox(geoid);
              }
              else
              {
                bbox = bbox.Union(segment.GetBoundingBox(geoid));
              }
            }
          }

          return bbox;
        }
      }

      return Rectangle<2>(false);
    }

    virtual double GetDistance(const Rectangle<2>& rect,
                               const Geoid *geoid = nullptr) const
    {
      assert(geoid == nullptr);

      double dist = std::numeric_limits<double>::max();

      for (const SimpleHalfSegment &segment : *this)
      {
        if (segment.ldp)
        {
          dist = std::min(dist, segment.GetDistance(rect));
        }
      }

      return dist;
    }

    virtual bool Intersects(const Rectangle<2>& rect,
                            const Geoid *geoid = nullptr) const
    {
      if (size != 0 && geoid == nullptr &&
          !GetBoundingBoxP()->Intersects(rect, geoid))
      {
        for (const SimpleHalfSegment &segment : *this)
        {
          if (segment.ldp && segment.Intersects(rect))
          {
            return true;
          }
        }

        return false;
      }

      assert(geoid == nullptr);

      throw SecondoException("Not implemented!");
    }

    virtual bool IsEmpty() const
    {
      return size == 0 || *GetCountP() == 0;
    }

  private:
    class SimplePoint 
    {
    public:
      SimplePoint()
      {
      }

      SimplePoint(double _x, double _y) :
        x(_x),y(_y)
      {
      }

      SimplePoint(const Point &point) :
        x(point.GetX()), y(point.GetY())
      {
      }

      bool Intersects(const Rectangle<2>& rect) const
      {
        const double left = rect.MinD(0),
          top = rect.MaxD(1),
          right = rect.MaxD(0),
          bottom = rect.MinD(1);

        return (x >= left && x <= right && y >= bottom && y <= top) ||
               AlmostEqual(x, left) || AlmostEqual(x, right) ||
               AlmostEqual(y, bottom) || AlmostEqual(y, top);
      }

      double GetDistance(const SimplePoint &other, Geoid *geoid = nullptr) const
      {
        if (geoid == nullptr)
        {
          return std::sqrt(std::pow(other.x - x, 2) + std::pow(other.y - y, 2));
        }

        double bearInitial,
          bearFinal;

        bool ok;

        const double d = ToPoint().DistanceOrthodromePrecise(other.ToPoint(),
                                                             *geoid, ok,
                                                             bearInitial,
                                                             bearFinal);

        assert(ok);

        return d;
      }

      bool operator == (const SimplePoint &other) const
      {
        return AlmostEqual(x, other.x) &&
               AlmostEqual(y, other.y);
      }

      bool operator != (const SimplePoint &other) const
      {
        return !AlmostEqual(x, other.x) ||
               !AlmostEqual(y, other.y);
      }

      bool operator < (const SimplePoint &other) const
      {
        if (!AlmostEqual(x, other.x))
        {
          return x < other.x;
        }

        if (!AlmostEqual(y, other.y))
        {
          return y < other.y;
        }

        return false;
      }

      bool operator <= (const SimplePoint &other) const
      {
        if (!AlmostEqual(x, other.x))
        {
          return x < other.x;
        }

        if (!AlmostEqual(y, other.y))
        {
          return y < other.y;
        }

        return true;
      }

      bool operator > (const SimplePoint &other) const
      {
        if (!AlmostEqual(x, other.x))
        {
          return x > other.x;
        }

        if (!AlmostEqual(y, other.y))
        {
          return y > other.y;
        }

        return false;
      }

      bool operator >= (const SimplePoint &other) const
      {
        if (!AlmostEqual(x, other.x))
        {
          return x > other.x;
        }

        if (!AlmostEqual(y, other.y))
        {
          return y > other.y;
        }

        return true;
      }

      Point ToPoint() const
      {
        return Point(true, x, y);
      }

      double x;
      double y; 

    };

    class SimpleHalfSegment
    {
    public:
      SimplePoint lp,
        rp;

      bool ldp;

      SimpleHalfSegment()
      {
      }

      SimpleHalfSegment(const SimplePoint &lp, const SimplePoint &rp,
                        bool ldp) :
        lp(lp),
        rp(rp),
        ldp(ldp)
      {
      }

      SimpleHalfSegment(const Point &lp, const Point &rp, bool ldp) :
        lp(lp.GetX(), lp.GetY()),
        rp(rp.GetX(), rp.GetY()),
        ldp(ldp)
      {
      }

      SimpleHalfSegment(const HalfSegment &hs) :
        SimpleHalfSegment(hs.GetLeftPoint(), hs.GetRightPoint(),
                          hs.IsLeftDomPoint())
      {
      }

      int Compare(const SimpleHalfSegment &other) const
      {
        const SimplePoint &dpA = ldp ? lp : rp,
          spA = ldp ? rp : lp,
          dpB = other.ldp ? other.lp : other.rp,
          spB = other.ldp ? other.rp : other.lp;

        if (dpA < dpB)
        {
          return -1;
        }
        else if (dpA > dpB)
        {
          return 1;
        }

        if (ldp != other.ldp)
        {
          return ldp ? 1 : -1;
        }
        else
        {
          const bool vA = AlmostEqual(lp.x, rp.x),
            vB = AlmostEqual(other.lp.x, other.rp.x);

          if (vA) // is a vertical?
          {
            if (vB) // are both vertical?
            {
              if (!AlmostEqual(spA.y, dpA.y))
              {
                if (spA.y > dpA.y)
                {
                  if (!AlmostEqual(spB.y, dpB.y) && spB.y > dpB.y)
                  {
                    return spA == spB ? 0 : spA < spB ? -1 : 1;
                  }

                  return ldp ? 1 : -1;
                }

                if (!AlmostEqual(spB.y, dpB.y) && spB.y < dpB.y)
                {
                  return spA == spB ? 0 : spA < spB ? -1 : 1;
                }
              }

              return ldp ? -1 : 1;
            }

            if (!AlmostEqual(spA.y, dpA.y))
            {
              return spA.y > dpA.y ? (ldp ? 1 : -1) : (ldp ? -1 : 1);
            }
          }
          else if (vB) // is only b vertical?
          {
            if (!AlmostEqual(spB.y, dpB.y))
            {
              return spB.y > dpB.y ? (ldp ? -1 : 1) : (ldp ? 1 : -1);
            }
          }

          const double kA = (dpA.y - spA.y) / (dpA.x - spA.x),
            kB = (dpB.y - spB.y) / (dpB.x - spB.x);

          if (!AlmostEqual(kA, kB))
          {
            return kA > kB ? 1 : -1;
          }

          return spA == spB ? 0 : spA < spB ? -1 : 1;
        }
      }

      Rectangle<2> GetBoundingBox(const Geoid* geoid = nullptr) const
      {
        if (geoid == nullptr) // euclidean case
        {
          const double
            minx = std::min(lp.x, rp.x),
            maxx = std::max(lp.x, rp.x),
            miny = std::min(lp.y, rp.y),
            maxy = std::max(lp.y, rp.y);
          double minMax[] = {minx - ApplyFactor(minx),
                              maxx + ApplyFactor(maxx),
                              miny - ApplyFactor(miny),
                              maxy + ApplyFactor(maxy)};
          return Rectangle<2>(true,minMax);
        }

        // spherical case
        const Rectangle<2> geobbox = lp.ToPoint().GeographicBBox(rp.ToPoint(),
                                                                 *geoid);

        const double
          minx = geobbox.MinD(0) - ApplyFactor(geobbox.MinD(0)),
          maxx = geobbox.MaxD(0) + ApplyFactor(geobbox.MaxD(0)),
          miny = geobbox.MinD(1) - ApplyFactor(geobbox.MinD(1)),
          maxy = geobbox.MaxD(1) + ApplyFactor(geobbox.MaxD(1));
        double minMax[] = {minx >= -180.0 ? minx : -180.0,
                            maxx <= 180.0 ? maxx : 180.0,
                            miny >= -90.0 ? miny : -90.0,
                            maxy <= 90.0 ? maxy : 90.0};
        return Rectangle<2>(true,minMax);
      }

      bool Intersects(const SimpleHalfSegment &other) const
      {
        if (!GetBoundingBox().Intersects(other.GetBoundingBox()))
        {
          return false;
        }

        double k, a, K, A;

        const double
          xl = lp.x,
          yl = lp.y,
          xr = rp.x,
          yr = rp.y,
          Xl = other.lp.x,
          Yl = other.lp.y,
          Xr = other.rp.x,
          Yr = other.rp.y;

        const bool vA = AlmostEqual(xl, xr),
          vB = AlmostEqual(Xl, Xr);

        if (vA && vB) // are both vertical?
        {
          return (AlmostEqual(xl, Xl) &&
                  (AlmostEqual(yl, Yl) ||
                   AlmostEqual(yl, Yr) ||
                   AlmostEqual(yr, Yl) ||
                   AlmostEqual(yr, Yr) ||
                   (yl > Yl && yl < Yr) || (yr > Yl && yr < Yr) ||
                   (Yl > yl && Yl < yr) || (Yr > yl && Yr < yr)));
        }

        if (!vA) // this segment is not vertical
        {
          k = (yr - yl) / (xr - xl);
          a = yl - k * xl;

          if (vB) //only other is vertical
          {
            const double y0 = k * Xl + a;

            return (Xl > xl || AlmostEqual(Xl, xl)) &&
                   (Xl < xr || AlmostEqual(Xl, xr)) &&
                   (((y0 > Yl || AlmostEqual(y0, Yl)) &&
                     (y0 < Yr || AlmostEqual(y0, Yr))) ||
                    ((y0 > Yr || AlmostEqual(y0, Yr)) &&
                     (y0 < Yl || AlmostEqual(y0, Yl))));
          }
        }

        if (!vB) // other segment is not vertical
        {
          K = (Yr - Yl) / (Xr - Xl);
          A = Yl - K * Xl;

          if (vB) // only this segment is vertical
          {
            const double Y0 = K * xl + A;

            return (xl > Xl || AlmostEqual(xl, Xl)) &&
                   (xl < Xr || AlmostEqual(xl, Xr)) &&
                   (((Y0 > yl || AlmostEqual(Y0, yl)) &&
                     (Y0 < yr || AlmostEqual(Y0, yr))) ||
                    ((Y0 > yr || AlmostEqual(Y0, yr)) &&
                     (Y0 < yl || AlmostEqual(Y0, yl))));
          }
        }

        // both segments are non-vertical

        if (!AlmostEqual(k, K))
        {
          const double x0 = (A - a) / (k - K);

          return (x0 > xl || AlmostEqual(x0, xl)) &&
                 (x0 < xr || AlmostEqual(x0, xr)) &&
                 (x0 > Xl || AlmostEqual(x0, Xl)) &&
                 (x0 < Xr || AlmostEqual(x0, Xr));
        }

        // are the segments in the same straight line?
        return (AlmostEqual(A, a) && (((xl > Xl || AlmostEqual(xl, Xl)) &&
                                       (xl < Xr || AlmostEqual(xl, Xr))) ||
                                      ((Xl > xl || AlmostEqual(xl, Xl)) &&
                                       (Xl < xr || AlmostEqual(xr, Xl)))));
      }

      bool Intersects(const Rectangle<2>& rect,
                      const Geoid *geoid = nullptr) const
      {
        assert(geoid != nullptr); //Not implemented

        if (rect.IsDefined() && !rect.IsEmpty())
        {
          if (lp.Intersects(rect) || rp.Intersects(rect))
          {
            return true;
          }

          // check for intersection of the 4
          // segments of the rectangle
          const SimplePoint p1 = SimplePoint(rect.MinD(0), rect.MinD(1)),
            p2 = SimplePoint(rect.MaxD(0), rect.MinD(1)),
            p3 = SimplePoint(rect.MaxD(0), rect.MaxD(1)),
            p4 = SimplePoint(rect.MinD(0), rect.MaxD(1));

          return (Intersects(SimpleHalfSegment(p1, p2, true)) ||
                  Intersects(SimpleHalfSegment(p2, p3, true)) ||
                  Intersects(SimpleHalfSegment(p3, p4, true)) ||
                  Intersects(SimpleHalfSegment(p4, p1, true)));
        }

        return false;
      }

      double GetDistance(const SimplePoint& point,
                         const Geoid* geoid = nullptr) const
      {
        if (geoid == nullptr) // euclidean geometry
        {
          const double
            xl = lp.x,
            yl = lp.y,
            xr = rp.x,
            yr = rp.y,
            X = point.x,
            Y = point.y;

          if (xl == xr || yl == yr)
          {
            if (xl == xr) //is vertical
            {
              if ((yl <= Y && Y <= yr) || (yr <= Y && Y <= yl))
              {
                return fabs(X - xl);
              }

              return std::min(point.GetDistance(lp), point.GetDistance(rp));
            }

            //is horizontal line: (yl==yr)

            if (xl <= X && X <= xr)
            {
              return fabs(Y - yl);
            }
          }
          else
          {
            const double k = (yr - yl) / (xr - xl),
              a = yl - k * xl,
              xx = (k * (Y - a) + X) / (k * k + 1),
              yy = k * xx + a;
              Coord XX = xx,
              YY = yy;

            if (xl <= XX && XX <= xr)
            {
              return point.GetDistance(SimplePoint(XX, YY));
            }
          }

          return std::min(point.GetDistance(lp), point.GetDistance(rp));
        }

        return geodist::getDist(lp.x,lp.y,rp.x,rp.y,point.x,point.y,geoid);

      }

      double GetDistance(const SimpleHalfSegment& other,
                         const Geoid* geoid = nullptr) const
      {
        if (geoid == nullptr) // euclidean geometry
        {
          if (!Intersects(other))
          {
            return std::min(std::min(GetDistance(other.lp),
                                     GetDistance(other.rp)),
                            std::min(other.GetDistance(lp),
                                     other.GetDistance(rp)));
          }

          return 0.0;
        }

        assert(geoid == nullptr);

        throw SecondoException("Not implemented!");
      }

      double GetDistance(const Rectangle<2>& rect,
                         const Geoid* geoid = nullptr) const
      {
        assert(geoid == nullptr);

        if (lp.Intersects(rect) || rp.Intersects(rect))
        {
          return 0.0;
        }

        // both endpoints are outside the rectangle
        const double
          x0 = rect.MinD(0),
          y0 = rect.MinD(1),
          x1 = rect.MaxD(0),
          y1 = rect.MaxD(1);

        const SimplePoint p0 = SimplePoint(x0, y0),
          p1 = SimplePoint(x1, y0),
          p2 = SimplePoint(x1, y1),
          p3 = SimplePoint(x0, y1);

        double dist;

        if (p0 == p1)
        {
          dist = GetDistance(p0);
        }
        else
        {
          dist = GetDistance(SimpleHalfSegment(p0, p1, true));
        }

        if (AlmostEqual(dist, 0.0))
        {
          return 0.0;
        }

        if (p1 == p2)
        {
          dist = std::min(dist, GetDistance(p1));
        }
        else
        {
          dist = std::min(dist, GetDistance(SimpleHalfSegment(p1, p2, true)));
        }

        if (AlmostEqual(dist, 0.0))
        {
          return 0.0;
        }

        if (p2 == p3)
        {
          dist = std::min(dist, GetDistance(p2));
        }
        else
        {
          dist = std::min(dist, GetDistance(SimpleHalfSegment(p2, p3, true)));
        }

        if (AlmostEqual(dist, 0.0))
        {
          return 0.0;
        }

        if (p3 == p0)
        {
          dist = std::min(dist, GetDistance(p3));
        }
        else
        {
          dist = std::min(dist, GetDistance(SimpleHalfSegment(p3, p0, true)));
        }

        if (AlmostEqual(dist, 0.0))
        {
          return 0.0;
        }

        return dist;
      }

      HalfSegment ToHalfSegment() const
      {
        return HalfSegment(ldp, lp.ToPoint(), rp.ToPoint());
      }
    };

    int64_t *GetCountP()
    {
      return (int64_t*)data;
    }

    const int64_t *GetCountP() const
    {
      return (int64_t*)data;
    }

    Rectangle<2> *GetBoundingBoxP()
    {
      return (Rectangle<2>*)(data + sizeof(int64_t));
    }

    const Rectangle<2> *GetBoundingBoxP() const
    {
      return (Rectangle<2>*)(data + sizeof(int64_t));
    }

    SimpleHalfSegment *begin()
    {
      return (SimpleHalfSegment*)(GetBoundingBoxP() + 1);
    }

    SimpleHalfSegment *end()
    {
      return begin() + *GetCountP();
    }

    const SimpleHalfSegment *begin() const
    {
      return (SimpleHalfSegment*)(GetBoundingBoxP() + 1);
    }

    const SimpleHalfSegment *end() const
    {
      return begin() + *GetCountP();
    }

    const char *data;

    uint64_t size;
  };

  typedef SimpleSpatialVSAttrArray<LineEntry> Lines;
}
