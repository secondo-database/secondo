/*

----
This file is part of SECONDO.

Copyright (C) 2004, University in Hagen, Department of Computer Science,
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

//[_][\_]
//[&][\&]
//characters [1] verbatim:   [\verb@]    [@]
//[TOC] [\tableofcontents]

""[1]

[1] Header File Histogram1d

December 2007, S. H[oe]cher, M. H[oe]ger, A. Belz, B. Poneleit


[TOC]

1 Overview

The file "HistogramUtils.h" contains declarations of helper functions
that are not attached to specific classes or operators.

2 Defines and includes

It includes "HistogramBase.h" which contains declarations of functions common
to both histogram1d and histogram2d.

*/

#ifndef HISTOGRAMUTILS_H_
#define HISTOGRAMUTILS_H_

#include "HistogramBase.h"
#include "AlmostEqual.h"

namespace hgr
{

/*
2 Functions and Constants

2.1 CmpReal(HIST[_]REAL, HIST[_]REAL)

Compares two real values using an absolute tolerance value
given by the constant FACTOR defined in the SECONDO environment.

returns 0 if a == b,

returns -1 if a $<$ b,

returns 1 if a $>$ b

*/
  inline int CmpReal(const HIST_REAL& a, const HIST_REAL& b)
  {
    if (fabs(a - b) < FACTOR)
      return 0;

    if (a < b)
      return -1;

    return 1;
  }

/*
2.2 AlmostEqual(HIST[_]REAL, HIST[_]REAL)

Tests two real values for equality using an absolute tolerance value
given by the constant FACTOR defined in the SECONDO environment.

returns true if a == b and false otherwise.

*/

  inline bool AlmostEqual(const HIST_REAL& a, const HIST_REAL& b)
  {
    return fabs(a - b) < FACTOR;
  }

/*
2.3 HashValue(HIST[_]REAL)

Hash function for HIST[_]REAL.
The algorithm is taken from the implementation of CcReal.

*/
  inline size_t HashValue(const HIST_REAL realval)
  {
    unsigned long h = 0;
    const char* s = (const char*)(&realval);
    for (unsigned int i = 1; i <= sizeof(HIST_REAL); i++)
    {
      h = 5 * h + *s;
      s++;
    }
    return size_t(h);
  }

/*
2.4 const size[_]t BITSIZE[_]SIZE[_]T[_]MINUS[_]1

This constant equals the number of bits of the datatype size[_]t minus 1.

*/

  const size_t BITSIZE_SIZE_T_MINUS_1 = sizeof(size_t) * CHAR_BIT - 1;

/*
2.5 Rotate(size[_]t)

Returns the bitwise by one left rotated parameter value.
Example: Rotate( 1110001 ) == 1100011

*/
  inline size_t Rotate(size_t val)
  {
    return (val << 1) | (val >> BITSIZE_SIZE_T_MINUS_1);
  }

} // namespace hgr

#endif /*HISTOGRAMUTILS_H_*/
