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

1 Interval.h

*/

#pragma once

#include "Algebras/Temporal/TemporalAlgebra.h"

namespace ColumnMovingAlgebra
{
/*
1.1 Declaration of struct ~Interval~

~Interval~ represents a time interval

*/
  struct Interval {
/*
~s~ and ~e~ represent the left and right boundary of the time interval.
~lc~ and ~rc~ determine, wether the interval is left closed respectively
right closed


*/  
    int64_t s, e;
    bool lc, rc;

/*
default constructor

*/    
    Interval() = default;
/*
constructor with explicit values

*/
    Interval(int64_t s, int64_t e, bool lc, bool rc);
/*
constructor for conversion from the time interval class of the Temporal algebra

*/
    Interval(temporalalgebra::Interval<Instant> i);
/*
~convert~ convertes the time interval to the representation used in the
Temporal algebra

*/
    temporalalgebra::Interval<Instant> convert();

/*
~length~ returns the duration of the time interval

*/
    int64_t length();
/*
~before~ returns true, iff ~instant~ is before the start of the time interval

*/
    bool before(int64_t instant);
/*
~after~ returns true, iff ~instant~ is after the end of the time interval

*/
    bool after(int64_t instant);
/*
~contains~ returns true, iff ~instant~ is in the time interval

*/
    bool contains(int64_t instant);
/*
~before~ returns true, iff this interval ends before ~b~ starts

*/
    bool before(Interval & b);
/*
~after~ returns true, iff this interval starts after ~b~ ends

*/
    bool after(Interval & b);
/*
~intersects~ returns true, iff the intersection of this interval and ~b~
is not empty

*/
    bool intersects(Interval & b);
/*
~intersection~ returns the intersection of this interval and ~b~

*/
    Interval intersection(Interval & b);
/*
~endsFirstComparedTo~ returns true, iff this interval ends before ~b~ ends

*/
    bool endsFirstComparedTo(Interval & b);
/*
~compare~ defines a ordering relation on time intervals.
returns -1 if this interval is smaller than b.
returns  0 if this interval is equal to b.
returns  1 if this interval is greater than b.

*/
    int compare(Interval b);
  };



/*
1.1 Implementation of struct ~Interval~

constructor with explicit values

*/
  inline Interval::Interval(int64_t s, int64_t e, bool lc, bool rc) :
    s(s),
    e(e),
    lc(lc),
    rc(rc)
  {
  }
/*
constructor for conversion from the time interval class of the Temporal algebra

*/
  inline Interval::Interval(temporalalgebra::Interval<Instant> i) :
    s(i.start.millisecondsToNull()),
    e(i.end.millisecondsToNull()),
    lc(i.lc),
    rc(i.rc)
  {
  }

/*
~convert~ convertes the time interval to the representation used in the
Temporal algebra

*/
  inline temporalalgebra::Interval<Instant> Interval::convert()
  {
    return temporalalgebra::Interval<Instant>(s, e, lc, rc);
  }

/*
~length~ returns the duration of the time interval

*/
  inline int64_t ColumnMovingAlgebra::Interval::length()
  {
    return e - s;
  }

/*
~before~ returns true, iff ~instant~ is before the start of the time interval

*/
  inline bool Interval::before(int64_t instant)
  {
    return e < instant || (e == instant && !rc);
  }

/*
~after~ returns true, iff ~instant~ is after the end of the time interval

*/
  inline bool Interval::after(int64_t instant)
  {
    return s > instant || (s == instant && !lc);
  }

/*
~contains~ returns true, iff ~instant~ is in the time interval

*/
  inline bool ColumnMovingAlgebra::Interval::contains(int64_t instant)
  {
    return !before(instant) && !after(instant);
  }

/*
~before~ returns true, iff this interval ends before ~b~ starts

*/
  inline bool Interval::before(Interval & b)
  {
    return e < b.s || (e == b.s && (!rc || !b.lc));
  }

/*
~after~ returns true, iff this interval starts after ~b~ ends

*/
  inline bool Interval::after(Interval & b)
  {
    return s > b.e || (s == b.e && (!lc || !b.rc));
  }

/*
~intersects~ returns true, iff the intersection of this interval and ~b~
is not empty

*/
  inline bool Interval::intersects(Interval & b)
  {
    return !before(b) && !after(b);
  }

/*
~intersection~ returns the intersection of this interval and ~b~

*/
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

/*
~endsFirstComparedTo~ returns true, iff this interval ends before ~b~ ends

*/
  inline bool Interval::endsFirstComparedTo(Interval & b)
  {
    return e < b.e || (e == b.e && !rc && b.rc);
  }

/*
~compare~ defines a ordering relation on time intervals.
returns -1 if this interval is smaller than b.
returns  0 if this interval is equal to b.
returns  1 if this interval is greater than b.

*/
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
