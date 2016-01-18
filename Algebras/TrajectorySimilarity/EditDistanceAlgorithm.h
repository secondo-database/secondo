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

[1] Interface of the Edit Distance (ED) Algorithm

\tableofcontents

\noindent

1 Introduction

This file declares functions that determine the Edit Distance between two
sequences as described in \cite[fig. 2]{M86}.

The Edit Distance

        $dist_{ED} : sequence \times sequence\ \rightarrow int$

is defined as the number of INSERT and DELETE operations needed to transform the
first sequence into the second sequence.

Other definitions of the Edit Distance also allow for REPLACE operations, but
such definitions are not considered here.


2 Includes, Constants

*/

#ifndef __EDIT_DISTANCE_ALGORITHM_H__
#define __EDIT_DISTANCE_ALGORITHM_H__

#include "functional"


namespace tsa {

/*
3 Declaration of Edit Distance functions

Type of a match function that returns ~true~, if the item at position $x \in
\lbrack 0,\ m\lbrack$ of the first sequence (having $m$ items) and the item at
position $y \in \lbrack0,\ n\lbrack$ of the second sequence (having $n$ items)
match; false otherwise.

*/
using ed_match_t = std::function<bool(const size_t x, const size_t y)>;

/*
Calculate the Edit Distance $d$ of two sequences, where $m$ is the length of the
first sequence, $n$ is the length of the second sequence, and $match$ is a match
function as described above. The result $d$ is in the range $[0,\ m+n]$.

The time complexity is $O((m+n)d)$ and the space complexity is $O(\max(m, n))$.

*/
size_t edit_distance(size_t m, size_t n, ed_match_t match);

/*
Calculate the Edit Distance $d$ as above, but return $-1$, if $d$ would exceed
$max$. If $max \ge m+n$, the result is never $-1$. Using this overload with
$max = m+n$ is equivalent to calling the first overload, except for the return
type.

The time complexity is $O((m+n)max)$ and the space complexity is $O(max)$.

*/
ssize_t edit_distance(size_t m, size_t n, ed_match_t match, size_t max);

} //-- namespace tsa

#endif  //-- __EDIT_DISTANCE_ALGORITHM_H__

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
