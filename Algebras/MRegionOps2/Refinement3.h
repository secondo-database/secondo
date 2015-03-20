/*
----
This file is part of SECONDO.

Copyright (C) 2013, University in Hagen, 
Faculty of Mathematics and Computer Science,
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

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]
//[ue] [\"u]
//[ae] [\"a]
//[oe] [\"o]

[1] File Refinement3.h

This file contains classes and functions for use within the operators intersection and inside

[2] Implementation with exakt dataype

Oktober 2014 - Maerz 2015, S. Schroer for master thesis.

[TOC]

1 Helper classes and methods

1.1 Some forward declaration of helper methods

*/

#ifndef REFINEMENT3_H_
#define REFINEMENT3_H_

#include <gmp.h>
#include <gmpxx.h>

#include "MovingRegion2Algebra.h"

mpq_class instant2MPQ( const Instant i );

/*

1.1 Precise Time Interval class ~precTimeInterval~

*/

class precTimeInterval {

public:
  mpq_class start;
  mpq_class end;
  bool lc;
  bool rc;
  
inline precTimeInterval():
      start(0),
      end(0),
      lc(false),
      rc(false) {}

inline precTimeInterval(mpq_class s, mpq_class e, bool l, bool r) :
      start(s),
      end(e),
      lc(l),
      rc(r) {}

inline precTimeInterval(Interval<Instant> t): 
      start(instant2MPQ(t.start)),
      end(instant2MPQ(t.end)),
      lc(t.lc),
      rc(t.rc) {}

inline precTimeInterval(Interval<Instant> t, PreciseInterval p, 
                          const DbArray<int>* preciseInstants): 
      start(0),
      end(0),
      lc(t.lc),
      rc(t.rc)
  {
    start = p.GetPreciseInitialInstant(preciseInstants);
    start = start + instant2MPQ(t.start);
    start.canonicalize();
    
    end = p.GetPreciseFinalInstant(preciseInstants);
    end = end + instant2MPQ(t.end);
    end.canonicalize();
  }
  
  inline bool operator==(precTimeInterval pti) const
  {
    return (lc==pti.lc) && (rc==pti.rc) 
    && (cmp(start, pti.start) == 0) && (cmp(end, pti.end) == 0);
  }
};


/*
1.1 Precise UPoint class ~precUPoint~

holds the precise values calculated within the set operations

*/
class precUPoint {
  
public:
  precTimeInterval pti;
  mpq_class x0;
  mpq_class y0;
  mpq_class x1;
  mpq_class y1;
  UPoint up;
  
  inline precUPoint():
      x0(0),
      y0(0),
      x1(0),
      y1(0),
      up(0) { }

 inline precUPoint(precTimeInterval p, mpq_class px0, mpq_class py0, 
                    mpq_class px1, mpq_class py1, UPoint pp):
      pti(p.start, p.end, p.lc, p.rc),
      x0(px0),
      y0(py0),
      x1(px1),
      y1(py1),
      up(pp) { }

 inline precUPoint(mpq_class s, mpq_class e, bool l, bool r, mpq_class px0, 
                    mpq_class py0, mpq_class px1, mpq_class py1, UPoint pp):
      pti(s, e, l, r),
      x0(px0),
      y0(py0),
      x1(px1),
      y1(py1),
      up(pp) { }
  
};


/*
1.1 Precise UBool class ~precUBool~

holds the precise values calculated within the set operations

*/
class precUBool {
  
public:
  precTimeInterval pti;
  bool status;
  
  inline precUBool():
      status(false) { }

  inline precUBool(precTimeInterval p, bool st):
      pti(p.start, p.end, p.lc, p.rc),
      status(st) { }

  inline precUBool(mpq_class s, mpq_class e, bool l, bool r, bool st):
      pti(s, e, l, r),
      status(st) { }
  
};


/*
1 Class ~RefinementPartition3~

for set operations inside and intersection with MPoint and MRegion2

1.1 Class definition 

*/

class RefinementPartition3 {
private:

/*
Private attributes:

  * ~iv~: Array (vector) of sub-intervals, which has been calculated from the
    unit intervals of the ~Mapping~ instances.

  * ~vur~: Maps intervals in ~iv~ to indices of original units in first
    ~Mapping~ instance. A $-1$ values indicates that interval in ~iv~ is no
    sub-interval of any unit interval in first ~Mapping~ instance.

  * ~vup~: Same as ~vur~ for second mapping instance.

*/
    vector<precTimeInterval> iv;
    vector<int> vur;
    vector<int> vup;

/*
~AddUnit()~ is a small helper method to create a new interval from
~start~ and ~end~ instant and ~lc~ and ~rc~ flags and to add these to the
~iv~, ~vur~ and ~vup~ vectors.

*/
    void AddUnits(const precTimeInterval pti, const int urPos, 
                  const int upPos);
    void AddUnits(const int urPos, const int upPos, const mpq_class start, 
                  const mpq_class end, const bool lc, const bool rc);

public:

/*
The constructor creates the refinement partition from the two ~Mapping~
instances ~mr~ and ~mp~.

Runtime is $O(\max(n, m))$ with $n$ and $m$ the numbers of units in
~mr~ and ~mp~.

*Preconditions*: mr.IsDefined AND mp.IsDefiened()

*/

 RefinementPartition3( MRegion2& m1,  MRegion2& m2);

/*
Since the elements of ~iv~ point to dynamically allocated objects, we need
a destructor.

*/
 ~RefinementPartition3() {}

/*
Return the number of intervals in the refinement partition.

*/
 unsigned int Size(void);

/*
Return the interval and indices in original units of position $pos$ in
the refinement partition in the referenced variables ~civ~, ~ur~ and
~up~. Remember that ~ur~ or ~up~ may be $-1$ if interval is no sub-interval
of unit intervals in the respective ~Mapping~ instance.

Runtime is $O(1)$.

*/
 void Get(const unsigned int pos, precTimeInterval& civ, int& ur, int& up);
};

#endif

