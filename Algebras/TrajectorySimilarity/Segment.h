/*
----
This file is part of SECONDO.

Copyright (C) 2015, University in Hagen, Department of Computer Science,
Database Systems for New Applications.

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
//paragraph [1] Title: [{\Large \bf \begin{center}] [\end{center}}]

[1] Segment Class Templates

\tableofcontents

\noindent

1 Introduction

This file declares and implements the class templates ~Segment$<$T$>$~ for
segments of points and ~SegmentIt$<$SEQ$>$~ for segment iterators on point
sequences.


2 Includes, Constants, and Forward Declarations

*/

#ifndef __SEGMENT_H__
#define __SEGMENT_H__


#include "HalfSegment.h"  // HalfSegment (from SpatialAlgebra)
#include "Point.h"        // Point (from SpatialAlgebra)


namespace tsa {

template<class T>
class Sequence;

class Point;
class TPoint;


/*
3 Class Template ~Segment$<$T$>$~

This class template implements segments of two points, ~start~ and ~end~, for
the point types ~Point~ and ~TPoint~.

3.1 Declaration and Implementation

*/
template<class T>
class Segment
{
public:
/*
Point type of the segment.

*/
  using point_t = T;

/*
Constructors and destructors.

*/
  Segment() = default;
  Segment(const T& start, const T& end) : start(start), end(end) { }
  Segment(const Segment& seg) : start(seg.start), end(seg.end) { }
  ~Segment() = default;

/*
Assignment operator.

*/
  Segment &operator=(const Segment& rhs)
  { start = rhs.start; end = rhs.end; return *this; }

/*
Get the start or end point of the segment.

*/
  const T& getStart() const { return start; }
  const T& getEnd() const { return end; }

/*
Determine the first barycentric coordinate of the point ~pt~ with respect to the
start and end point of the segment. If the length of the segment is 0
(i.\ e.\ $start = end$) or if ~pt~ is not on the line defined by the segment's
end points, the result is indefinite.

If ~pt~ is on the segment itself, the result is in the interval $[0, 1]$; if
~pt~ is before the start of the segment, the result is $<0$; otherwise ~pt~ is
after the end of the the segment and the result is $>1$.

The second barycentric coordinate of ~pt~ is $1 - baryCoord(pt)$, and

        $pt = (baryCoord(pt) \cdot start) + ((1 - baryCoord(pt)) \cdot end)$.

*/
  double baryCoord(const Point& pt) const
  {
    const Point p_start(start);
    const Point p_end(end);

    const double dist1 = euclideanDistance(p_start, pt);
    const double dist2 = euclideanDistance(p_end, pt);
    const double len   = euclideanDistance(p_start, p_end);

/*
If ~pt~ is on the segment or after its end, the result is positive; otherwise
it is negative.

*/
    if ((dist1 <= len && dist2 <= len) || dist2 < dist1)
      return dist1 / len;

    return -(dist1 / len);
  }

/*
Get a ~HalfSegment~ object of the SpatialAlgebra that represents the segment.

*/
  const HalfSegment toHalfSegment() const
  { return HalfSegment(/*ldm*/ true, start.toPoint(), end.toPoint()); }

/*
Get a string representation for display purposes.

*/
  std::string toString() const
  {
    std::ostringstream os;
    os << "(" << start.ToString() << ", " << end.toString() << ")";
    return os.str();
  }

private:
  T start = T();
  T end = T();
};


/*
3.2 Non-member Functions

Check if two segments intersect and if so, determine the point where they cross.
A $geoid \ne nullptr$ is taken into account, if implemented in the functions of
the SpatialAlgebra used here.

*/
template<class T>
bool intersection(
    const Segment<T>& seg1, const Segment<T>& seg2, Point& cross,
    const Geoid* geoid = nullptr)
{
  HalfSegment hs1 = seg1.toHalfSegment();
  HalfSegment hs2 = seg2.toHalfSegment();
  ::Point pt;
  if (!hs1.Intersection(hs2, pt, geoid))
    return false;
  cross = pt;
  return true;
}


/*
4 Class Template ~SegmentIt$<$SEQ$>$~

This class template provides segment iterators for the point sequence classes
~PointSeq~ and ~TPointSeq~.

An object of the class ~SegmentIt$<$SEQ$>$~ either points at a segment defined
by two consecutive points in the point sequence or at the place one past the
last segment in the sequence. A sequence with $n > 1$ points has $n-1$ segments
with index numbers in the interval $[0,\ n-2]$. A sequence with $n \le 1$ points
has no segments.

*/
template<class SEQ>
class SegmentIt
{
public:
/*
Point and segment type.

*/
  using point_t = typename SEQ::point_t;
  using segment_t = Segment<typename SEQ::point_t>;

/*
Create a segment iterator for the first segment of the sequence. If the sequence
contains less than one item, this is the same as $end(seq)$.

*/
  static SegmentIt begin(const SEQ& seq)
  { return SegmentIt(seq, 0); }

/*
Get a segment iterator for one past the last segment of the sequence.

*/
  static SegmentIt end(const SEQ& seq)
  {
    return SegmentIt(
        seq, (seq.GetNoComponents() > 0) ? seq.GetNoComponents() - 1 : 0);
  }

/*
Constructors and destructors.

*/
  SegmentIt() = delete;
  SegmentIt(const SEQ &s, std::size_t p) : seq(s), pos(p) { }
  SegmentIt(const SegmentIt& rhs) : seq(rhs.seq), pos(rhs.pos) { }
  ~SegmentIt() = default;

/*
The assignment operators work only on iterators of the same sequence object.

*/
  SegmentIt &operator=(const SegmentIt& rhs)
  { assert(&seq == &rhs.seq); pos = rhs.pos; return *this; }
  SegmentIt &operator=(const SegmentIt&& rhs)
  { assert(&seq == &rhs.seq); pos = rhs.pos; return *this; }

/*
The increment and decrement operators do not perform range checks.

*/
  SegmentIt &operator++() { ++pos; return *this; }
  SegmentIt operator++(int) { SegmentIt tmp(*this); ++pos; return tmp; }
  SegmentIt operator+(std::size_t dif) const
  { SegmentIt tmp(*this); tmp.pos += dif; return tmp; }
  SegmentIt &operator--() { --pos; return *this; }
  SegmentIt operator--(int) { SegmentIt tmp(*this); --pos; return tmp; }
  SegmentIt operator-(std::size_t dif) const
  { SegmentIt tmp(*this); tmp.pos -= dif; return tmp; }

/*
Equality operators.

*/
  bool operator==(const SegmentIt &rhs) const
  { return &seq == &rhs.seq && rhs.pos == pos; }
  bool operator!=(const SegmentIt &rhs) const
  { return &seq != &rhs.seq || rhs.pos != pos; }

/*
Relational operators work only on iterators of the same sequence object.

*/
  bool operator>(const SegmentIt &rhs) const
  { assert(&seq == &rhs.seq); return pos > rhs.pos; }
  bool operator<(const SegmentIt &rhs) const
  { assert(&seq == &rhs.seq); return pos < rhs.pos; }

/*
The boolean operators test whether the iterator points at a segment ($operator
\ bool() == true$) or not ($operator\ bool() == false$).

*/
  explicit operator bool() const { return pos + 1 < seq.GetNoComponents(); }
  bool operator!() const { return pos + 1 >= seq.GetNoComponents(); }

/*
The dereference operators return a reference or a pointer to the current segment
represented. They may only be used, if the iterator points at a segment.

*/
  const segment_t &operator*() const
  {
    assert(operator bool());
    seg = segment_t(seq.get(pos), seq.get(pos+1));
    return seg;
  }
  const segment_t *operator->() const
  {
    assert(operator bool());
    seg = segment_t(seq.get(pos), seq.get(pos+1));
    return &seg;
  }

private:
  const SEQ &seq;
  std::size_t pos = 0;
  mutable segment_t seg;
};

} //-- namespace tsa

#endif  //-- "__SEGMENT_H__"
