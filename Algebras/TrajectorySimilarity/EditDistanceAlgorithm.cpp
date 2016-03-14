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
//paragraph [2] Filename: [{\tt \begin{center}] [\end{center}}]

[1] Implementation of the Edit Distance (ED) Algorithm

[2] EditDistanceAlgorithm.cpp

\tableofcontents

\noindent

1 Introduction

This file implements the Edit Distance algorithm.


2 Includes

*/

#include "EditDistanceAlgorithm.h"

#include <vector>


namespace tsa {

/*
3 Implementation of Edit Distance algorithm

Implementation of the Edit Distance algorithm as described in
\cite[fig. 2]{M86}.

*/
size_t edit_distance(size_t m, size_t n, ed_match_t match)
{
  return edit_distance(m, n, match, m + n);
}

ssize_t edit_distance(
    const size_t m, const size_t n, ed_match_t match, const size_t max)
{
/*
The original algorithm uses an array with the index interval $[-max, max]$.
Since C++ does not support negative indices, this implementation uses $max$ as
offset to map indices to the range $[0, 2max]$.

*/
  std::vector<size_t> v((2 * max) + 1);

  v[max + 1] = 0;
  for (ssize_t d = 0; d <= static_cast<ssize_t>(max); ++d) {
    for (ssize_t k = -d; k <= d; k += 2) {
      size_t x =
          ((k == -d) || (k != d && v[max + k - 1] < v[max + k + 1])) ?
              v[max + k + 1]  :  v[max + k - 1] + 1;
      size_t y = x - k;
      while (x < m && y < n && match(x, y)) {
        ++x; ++y;
      }
      if (x >= m && y >= n)
        return d;
      v[max + k] = x;
    }
  }

/*
This point is never reached, if $max \ge m+n$.

*/
  return -1;
}

} //-- namespace tsa

/*
\begin{thebibliography}{ABCD99}

//[Oe] [\"{O}]

\bibitem[M86]{M86}
Eugene W.\ Myers.
\newblock {An {O(ND)} Difference Algorithm and Its Variations}.
\newblock {\em Algorithmica}, 1(2): 251--266, 1986,
  http://dx.doi.org/10.1007/BF01840446;
  http://dblp.uni-trier.de/rec/bib/journals/algorithmica/Meyers86.

\end{thebibliography}

*/
