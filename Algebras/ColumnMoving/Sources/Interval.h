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

#include "TemporalAlgebra.h"

namespace ColumnMovingAlgebra
{
  struct Interval {
    int64_t s, e;
    bool lc, rc;

    Interval() = default;
    Interval(int64_t s, int64_t e, bool lc, bool rc);
    Interval(temporalalgebra::Interval<Instant> i);
    temporalalgebra::Interval<Instant> convert();

    int64_t length();
    bool before(int64_t instant);
    bool after(int64_t instant);
    bool contains(int64_t instant);
    bool before(Interval & b);
    bool after(Interval & b);
    bool intersects(Interval & b);
    Interval intersection(Interval & b);
    bool endsFirstComparedTo(Interval & b);
    int compare(Interval b);
  };



  inline Interval::Interval(int64_t s, int64_t e, bool lc, bool rc) :
    s(s),
    e(e),
    lc(lc),
    rc(rc)
  {
  }

  inline Interval::Interval(temporalalgebra::Interval<Instant> i) :
    s(i.start.millisecondsToNull()),
    e(i.end.millisecondsToNull()),
    lc(i.lc),
    rc(i.rc)
  {
  }

  inline temporalalgebra::Interval<Instant> Interval::convert()
  {
    return temporalalgebra::Interval<Instant>(s, e, lc, rc);
  }

  inline int64_t ColumnMovingAlgebra::Interval::length()
  {
    return e - s;
  }

  inline bool Interval::before(int64_t instant)
  {
    return e < instant || (e == instant && !rc);
  }

  inline bool Interval::after(int64_t instant)
  {
    return s > instant || (s == instant && !lc);
  }

  inline bool ColumnMovingAlgebra::Interval::contains(int64_t instant)
  {
    return !before(instant) && !after(instant);
  }

  inline bool Interval::before(Interval & b)
  {
    return e < b.s || (e == b.s && (!rc || !b.lc));
  }

  inline bool Interval::after(Interval & b)
  {
    return s > b.e || (s == b.e && (!lc || !b.rc));
  }

  inline bool Interval::intersects(Interval & b)
  {
    return !before(b) && !after(b);
  }

  inline Interval Interval::intersection(Interval & b)
  {
    Interval i;

    if (s > b.s) {
      i.s = s;
      i.lc = lc;
    }
    else if (s < b.s) {
      i.s = b.s;
      i.lc = b.lc;
    } else {
      i.s = s;
      i.lc = lc && b.lc;
    }

    if (e < b.e) {
      i.e = e;
      i.rc = rc;
    } else if (e > b.e) {
      i.e = b.e;
      i.rc = b.rc;
    } else {
      i.e = e;
      i.rc = rc && b.rc;
    }

    return i;
  }

  inline bool Interval::endsFirstComparedTo(Interval & b)
  {
    return e < b.e || (e == b.e && !rc && b.rc);
  }

  inline int Interval::compare(Interval b)
  {
    int64_t tDiff;

    tDiff = s - b.s;
    if (tDiff != 0)
      return tDiff < 0 ? -1 : 1;

    tDiff = e - b.e;
    if (tDiff != 0)
      return tDiff < 0 ? -1 : 1;

    if (lc != b.lc)
      return lc ? -1 : 1;

    if (rc != b.rc)
      return rc ? -1 : 1;

    return 0;
  }
}
