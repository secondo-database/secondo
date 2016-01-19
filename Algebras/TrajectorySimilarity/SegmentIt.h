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

[1] Segment Iterator Class Template

\tableofcontents

\noindent

1 Introduction

This file declares and implements the class template ~SegmentIt$<$SEQ$>$~ for
segment iterators on point sequences.


2 Includes, Constants, and Forward Declarations

*/

#ifndef __SEGMENT_IT_H__
#define __SEGMENT_IT_H__

#include "HalfSegment.h"  // HalfSegment (from SpatialAlgebra)


namespace tsa {

template<class T>
class Sequence;


/*
3 Class Template ~SegmentIt$<$SEQ$>$~

This class template provides segment iterators for the point sequence classes
~PointSeq~ and ~TPointSeq~.

An object of the class ~SegmentIt$<$SEQ$>$~ either points at a segment defined
by two consecutive points in the point sequence or at the place one past the
last segment in the sequence. A sequence with $n > 1$ points has $n-1$ segments
with index numbers in the range $[0,\ n-2]$. A sequence with $n \le 1$ points
has no segments.

*/
template<class SEQ>
class SegmentIt
{
public:
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
Constructors and desctructors.

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
The increment and decrement operators do not check the ranges.

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
represented as a ~HalfSegment~ object of the SpatialAlgebra. They may only be
used, if the iterator points at a segment.

*/
  const HalfSegment &operator*() const
  {
    assert(operator bool());
    hs = HalfSegment(
        /*ldm*/ true, seq.get(pos).toPoint(), seq.get(pos+1).toPoint());
    return hs;
  }
  const HalfSegment *operator->() const
  {
    assert(operator bool());
    hs = HalfSegment(
        /*ldm*/ true, seq.get(pos).toPoint(), seq.get(pos+1).toPoint());
    return &hs;
  }

private:
  const SEQ &seq;
  std::size_t pos = 0;
  mutable HalfSegment hs;
};

} //-- namespace tsa

#endif  //-- __SEGMENT_IT_H__
