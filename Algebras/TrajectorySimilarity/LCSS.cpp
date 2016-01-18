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

[1] Implementation of the Longest Common Subsequence (LCSS) Operators

\tableofcontents

\noindent

1 Introduction

This file implements and registers Longest Common Subsequence operators for
point sequences as described in \cite{VGK02} and \cite[section 2]{COO05}.

A sequence $S$ is transformed into a subsequence $S'$ by removing an arbitrary
number of items, \cite[page 292]{Aho90}. For instance, in terms of characters,
IIAI is a subsequence of SIMILARITY, and so are SIMILARITY (where no items are
removed) and the empty string (where all items are removed).

In general, a subsequence $S'$ is common to two sequences $P, Q$, if $S'$ is a
subsequence of $P$ and also of $Q$. This definition fits for discrete domains,
but for continuous domains it is usually relaxed as follows: $P'$, a subsequence
of $P$, is considered a common subsequence of $P$ and $Q$, if it ~matches~ a
subsequence $Q'$ of $Q$. Two subsequences
$P'= \langle p'_0,\ \dots,\ p'_{n-1}\rangle$ and
$Q'= \langle q'_0,\ \dots,\ q'_{m-1}\rangle$ match, if they have the same number
of items $n=m$ and for each $i \in \lbrack0,\ n \lbrack$ the item $p'_i$ matches
$q'_i$ within some neighbourhood.

For point sequences, \cite[section 2]{COO05} defines that two points $p, q$
match, if their spatial $L^1$ distance is less than $epsilon > 0$, that is

        $(|p.x-q.x| < \epsilon) \lor (|p.y-q.y| < \epsilon).$

In addition, \cite{VGK02} requires that the index positions $i, j$ of the points
$p, q$ in their original sequences $P, Q$ are at most $\delta \ge 0$ places
apart, that is $|i-j| \le \delta$.


1.1 Operators

The operators

        $lcss : SEQ \times SEQ \times real\ [\times\ int] \rightarrow int$,
        \newline
        $rel\_lcss : SEQ \times SEQ \times real\ [\times\ int] \rightarrow int$,
        and \newline
        $dist\_lcss : SEQ \times SEQ \times real\ [\times\ int] \rightarrow int$

with $SEQ \in \{pointseq,\ tpointseq\}$ measure the similarity or the distance
of two point sequences based on the length of the Longest Common Subsequence.

If $SEQ = tpointseq$, the temporal component is ignored.

The overloads

        $\circ : SEQ \times SEQ \times real \rightarrow int$

with $\circ(P,\ Q,\ \epsilon)$ consider two points matching, if their $L^1$
distance is less than $\epsilon$, as described in \cite[section 2]{COO05}.

The overloads

        $\circ : SEQ \times SEQ \times real \times int \rightarrow int$

with $\circ(P,\ Q,\ \epsilon,\ \delta)$ consider two points matching, if their
$L^1$ distance is less than $\epsilon$ and their index positions in the original
sequences are at most $\delta$ places apart, as described in \cite{VGK02}.

1.1.1 ~lcss~

The operator ~lcss~ determines the absolute length of the Longest Common
Subsequence of two point sequences. The result is in the range
$[0,\ \min(m,\ n)]$, where $m,\ n$ are the lengths of the input sequences and a
greater result indicates higher similarity. If any of the two sequences is
~undefined~, the result is ~undefined~.

1.1.2 ~rel\_lcss~

The operator ~rel\_lcss~ determines the relative length of the Longest Common
Subsequence of two point sequences, that is the absolute length of the LCSS
divided by the length of the shorter of the two sequences. \cite[def. 2]{VGK02}
presents this as the similarity function $S1$. The result is in the range
$[0,\ 1]$, where a value of 0 indicates low similarity and a value of 1
indicates that the shorter sequence is a subsequence of the longer sequence. If
any of the two sequences is ~undefined~ or empty (i.e. it contains no point),
the result is ~undefined~.

1.1.3 ~dist\_lcss~

The operator ~dist\_lcss~ measures the distance of two point sequences in the
range $[0,\ 1]$. It is defined as $dist\_lcss = 1 - rel\_lcss$.
\cite[def. 4]{VGK02} presents this as the distance function $D1$. If
any of the two sequences is ~undefined~ or empty (i.e. it contains no point),
the result is ~undefined~.


2 Includes

*/

#include "EditDistanceAlgorithm.h"
#include "PointSeq.h"
#include "TrajectorySimilarity.h"
#include "VectorTypeMapUtils.h"


namespace tsa {

/*
3 Registration of Operators

3.1 Common declarations

The operators ~rel\_lcss~ and ~dist\_lcss~ have the same signature and the same
requirements on their parameters. Therefore they can use common value mapping,
type mapping, and select functions.

*/
template<class SEQ>
using lcss_func_t = double (*)(
    const SEQ&, const SEQ&, const double epsilon, const int delta);

template<class SEQ, bool HAS_DELTA, lcss_func_t<SEQ> FUNC>
int LCSSOpValueMap(
    Word* args, Word& result, int /*message*/, Word& /*local*/, Supplier s);


/*
3.2 ~lcss~

This function expects two defined sequences, an $epsilon > 0$ and optionally a
$delta \ge 0$.

As shown in \cite[page 292]{Aho90}, the relationship between the Edit Distance
$D_{ED}(P,\ Q)$ and the length of the Longest Common Subsequence $LCSS(P,\ Q)$
of two sequences $P,\ Q$ with lengths $m,\ n$, respectively, is

        $D_{ED}(P,\ Q) = m + n - 2 LCSS(P,\ Q)$.

The following implementation first determines the Edit Distance and then derives
the length of the Longest Common Subsequence.

*/
template<class SEQ>
unsigned int lcss(
    const SEQ& seq1, const SEQ& seq2,
    const double epsilon, const int delta = -1)
{
/*
Determine the Edit Distance of the two sequences with the match function
specified as a lambda function.

*/
  const unsigned int ed = edit_distance(
      seq1.size(), seq2.size(),
      [&](const unsigned int i1, const unsigned int i2) -> bool {
/*
It would be a bug in the algorithm's implementation, is one of the indices was
out of range.

*/
        assert(i1 < seq1.size() && i2 < seq2.size());

/*
If $delta \ne -1$, check for maximum distance of items in original
sequences.

*/
        if (delta != -1 && abs(i1 - i2) > delta)
          return false;

/*
Check for maximum spatial distance.

*/
        const Point p1 = seq1.get(i1);
        const Point p2 = seq2.get(i2);
        return l1Distance(p1, p2) < epsilon;
      }
  );

/*
Derive the LCSS length from the Edit Distance.

*/
  const unsigned int max_edits = seq1.size() + seq2.size();
  return (max_edits - ed) / 2;
}

/*
The signature of the operator ~lcss~ differs from the signatures of the other
operators. Therefore it has its own value mapping, type mapping, and select
functions.

*/
template<class SEQ, bool HAS_DELTA>
int LCSSValueMap(
    Word* args, Word& result, int /*message*/, Word& /*local*/, Supplier s)
{
  const SEQ& seq1 = *static_cast<SEQ*>(args[0].addr);
  const SEQ& seq2 = *static_cast<SEQ*>(args[1].addr);
  const CcReal &cc_epsilon = *static_cast<CcReal*>(args[2].addr);
  const CcInt *cc_delta =
      HAS_DELTA ? static_cast<CcInt*>(args[3].addr) : nullptr;
  result = qp->ResultStorage(s);    // CcInt
  CcInt& sim = *static_cast<CcInt*>(result.addr);

/*
Require defined sequences, a defined $epsilon$ and a defined $delta$, if
specified.

*/
  if (!seq1.IsDefined() || !seq2.IsDefined() || !cc_epsilon.IsDefined() ||
      (HAS_DELTA && !cc_delta->IsDefined())) {
    sim.SetDefined(false);
    return 0;
  }

/*
Require a positive $epsilon$.

*/
  const double epsilon = cc_epsilon.GetValue();
  if (epsilon < 0.0) {
    sim.SetDefined(false);
    return 0;
  }

/*
Require a non-negative $delta$, if specified.

*/
  const int delta = HAS_DELTA ? cc_delta->GetValue() : -1;
  if (HAS_DELTA && delta < 0) {
    sim.SetDefined(false);
    return 0;
  }

  sim.Set(lcss(seq1, seq2, epsilon, delta));
  return 0;
}

ValueMapping lcss_functions[] = {
  LCSSValueMap<PointSeq,  /*HAS_DELTA*/ false>,
  LCSSValueMap<PointSeq,  /*HAS_DELTA*/ true>,
  LCSSValueMap<TPointSeq, /*HAS_DELTA*/ false>,
  LCSSValueMap<TPointSeq, /*HAS_DELTA*/ true>,
  nullptr
};

struct LCSSInfo : OperatorInfo
{
  LCSSInfo() : OperatorInfo()
  {
    name      = "lcss";
    signature = PointSeq::BasicType() + " x " + PointSeq::BasicType() + " x "
                + CcReal::BasicType() + " -> " + CcInt::BasicType();
    appendSignature(
                PointSeq::BasicType() + " x " + PointSeq::BasicType() + " x "
                + CcReal::BasicType() + " x " + CcInt::BasicType() + " -> "
                + CcInt::BasicType());
    appendSignature(
                TPointSeq::BasicType() + " x " + TPointSeq::BasicType() + " x "
                + CcReal::BasicType() + " -> " + CcInt::BasicType());
    appendSignature(
                TPointSeq::BasicType() + " x " + TPointSeq::BasicType() + " x "
                + CcReal::BasicType() + " x " + CcInt::BasicType() + " -> "
                + CcInt::BasicType());
    syntax    = "lcss(_, _, _[, _])";
    meaning   = "Length of Longest Common Subsequence (LCSS) of two point "
                "sequences of lengths m, n. The result is in the range "
                "[0, min(m,n)], where a greater value indicates higher "
                "similarity. If any of the two sequences is undefined, the "
                "result is undefined.\n"
                "The overload lcss(P, Q, epsilon) considers two points "
                "matching, if they are within spatial L^1 distance epsilon. "
                "The overload lcss(P, Q, epsilon, delta) additionally requires "
                "that the indices of the two points in the original sequences "
                "P, Q are at most delta places apart.\n"
                "The time complexity is O((m+n)d) and the space complexity is "
                "O(max(m,n)), where d is the length of the LCSS.";
  }
};

const mappings::VectorTypeMaps lcss_maps = {
  /*0*/ {{PointSeq::BasicType(), PointSeq::BasicType(), CcReal::BasicType()},
    /* -> */ {CcInt::BasicType()}},
  /*1*/ {{PointSeq::BasicType(), PointSeq::BasicType(), CcReal::BasicType(),
    CcInt::BasicType()}, /* -> */ {CcInt::BasicType()}},
  /*2*/ {{TPointSeq::BasicType(), TPointSeq::BasicType(), CcReal::BasicType()},
    /* -> */ {CcInt::BasicType()}},
  /*3*/ {{TPointSeq::BasicType(), TPointSeq::BasicType(), CcReal::BasicType(),
    CcInt::BasicType()}, /* -> */ {CcInt::BasicType()}}
};

ListExpr LCSSTypeMap(ListExpr args)
{ return mappings::vectorTypeMap(lcss_maps, args); }

int LCSSSelect(ListExpr args)
{ return mappings::vectorSelect(lcss_maps, args); }


/*
3.3 ~rel\_lcss~

This function expects two defined and non-empty sequences, an $epsilon > 0$ and
optionally a $delta \ge 0$.

*/
template<class SEQ>
double rel_lcss(
    const SEQ& seq1, const SEQ& seq2,
    const double epsilon, const int delta = -1)
{
/*
Determine absolute length of Longest Common Subsequence.

*/
  const unsigned int l = lcss(seq1, seq2, epsilon, delta);

/*
The minimum sequence length is always positive, since both sequences are defined
and non-empty.

*/
  const unsigned int min_length = std::min(seq1.size(), seq2.size());

/*
The absolute LCSS is at most the minimum sequence length. Therefore the return
value is in the range $[0, 1]$.

*/
  return static_cast<double>(l) / static_cast<double>(min_length);
}

ValueMapping rel_lcss_functions[] = {
  LCSSOpValueMap<PointSeq,  /*HAS_DELTA*/ false, rel_lcss>,
  LCSSOpValueMap<PointSeq,  /*HAS_DELTA*/ true,  rel_lcss>,
  LCSSOpValueMap<TPointSeq, /*HAS_DELTA*/ false, rel_lcss>,
  LCSSOpValueMap<TPointSeq, /*HAS_DELTA*/ true,  rel_lcss>,
  nullptr
};

struct RelLCSSInfo : OperatorInfo
{
  RelLCSSInfo() : OperatorInfo()
  {
    name      = "rel_lcss";
    signature = PointSeq::BasicType() + " x " + PointSeq::BasicType() + " x "
                + CcReal::BasicType() + " -> " + CcReal::BasicType();
    appendSignature(
                PointSeq::BasicType() + " x " + PointSeq::BasicType() + " x "
                + CcReal::BasicType() + " x " + CcInt::BasicType() + " -> "
                + CcReal::BasicType());
    appendSignature(
                TPointSeq::BasicType() + " x " + TPointSeq::BasicType() + " x "
                + CcReal::BasicType() + " -> " + CcReal::BasicType());
    appendSignature(
                TPointSeq::BasicType() + " x " + TPointSeq::BasicType() + " x "
                + CcReal::BasicType() + " x " + CcInt::BasicType() + " -> "
                + CcReal::BasicType());
    syntax    = "rel_lcss(_, _, _[, _])";
    meaning   = "Relative length of Longest Common Subsequence (LCSS) of two "
                "point sequences. The result is in the range [0, 1], where a "
                "value of 0 indicates low similarity and a value of 1 "
                "indicates that the shorter sequence is a subsequence of the "
                "longer sequence. If any of the two sequences is undefined or "
                "empty, the result is undefined.\n"
                "The overload rel_lcss(P, Q, epsilon) considers two points "
                "matching, if they are within spatial L^1 distance epsilon. "
                "The overload rel_lcss(P, Q, epsilon, delta) additionally "
                "requires that the indices of the two points in the original "
                "sequences P, Q are at most delta places apart.\n"
                "The time complexity is O((m+n)d) and the space complexity is "
                "O(max(m,n)), where d is the length of the LCSS.";
  }
};


/*
3.4 ~dist\_lcss~

This function expects two defined and non-empty sequences, an $epsilon > 0$ and
optionally a $delta \ge 0$.

*/
template<class SEQ>
double dist_lcss(
    const SEQ& seq1, const SEQ& seq2,
    const double epsilon, const int delta = -1)
{
  return 1 - rel_lcss(seq1, seq2, epsilon, delta);
}

ValueMapping dist_lcss_functions[] = {
  LCSSOpValueMap<PointSeq,  /*HAS_DELTA*/ false, dist_lcss>,
  LCSSOpValueMap<PointSeq,  /*HAS_DELTA*/ true,  dist_lcss>,
  LCSSOpValueMap<TPointSeq, /*HAS_DELTA*/ false, dist_lcss>,
  LCSSOpValueMap<TPointSeq, /*HAS_DELTA*/ true,  dist_lcss>,
  nullptr
};

struct DistLCSSInfo : OperatorInfo
{
  DistLCSSInfo() : OperatorInfo()
  {
    name      = "dist_lcss";
    signature = PointSeq::BasicType() + " x " + PointSeq::BasicType() + " x "
                + CcReal::BasicType() + " -> " + CcReal::BasicType();
    appendSignature(
                PointSeq::BasicType() + " x " + PointSeq::BasicType() + " x "
                + CcReal::BasicType() + " x " + CcInt::BasicType() + " -> "
                + CcReal::BasicType());
    appendSignature(
                TPointSeq::BasicType() + " x " + TPointSeq::BasicType() + " x "
                + CcReal::BasicType() + " -> " + CcReal::BasicType());
    appendSignature(
                TPointSeq::BasicType() + " x " + TPointSeq::BasicType() + " x "
                + CcReal::BasicType() + " x " + CcInt::BasicType() + " -> "
                + CcReal::BasicType());
    syntax    = "dist_lcss(_, _, _[, _])";
    meaning   = "Distance of two point sequences based defined as 1 - "
                "rel_lcss. If any of the two sequences is undefined or empty, "
                "the result is undefined.\n"
                "The overload dist_lcss(P, Q, epsilon) considers two points "
                "matching, if they are within spatial L^1 distance epsilon. "
                "The overload dist_lcss(P, Q, epsilon, delta) additionally "
                "requires that the indices of the two points in the original "
                "sequences P, Q are at most delta places apart.\n"
                "The time complexity is O((m+n)d) and the space complexity is "
                "O(max(m,n)), where d is the length of the LCSS.";
  }
};


/*
3.5 Common implementation

*/
const mappings::VectorTypeMaps lcss_op_maps = {
  /*0*/ {{PointSeq::BasicType(), PointSeq::BasicType(), CcReal::BasicType()},
    /* -> */ {CcReal::BasicType()}},
  /*1*/ {{PointSeq::BasicType(), PointSeq::BasicType(), CcReal::BasicType(),
    CcInt::BasicType()}, /* -> */ {CcReal::BasicType()}},
  /*2*/ {{TPointSeq::BasicType(), TPointSeq::BasicType(), CcReal::BasicType()},
    /* -> */ {CcReal::BasicType()}},
  /*3*/ {{TPointSeq::BasicType(), TPointSeq::BasicType(), CcReal::BasicType(),
    CcInt::BasicType()}, /* -> */ {CcReal::BasicType()}}
};

ListExpr LCSSOpTypeMap(ListExpr args)
{ return mappings::vectorTypeMap(lcss_op_maps, args); }

int LCSSOpSelect(ListExpr args)
{ return mappings::vectorSelect(lcss_op_maps, args); }

template<class SEQ, bool HAS_DELTA, lcss_func_t<SEQ> FUNC>
int LCSSOpValueMap(
    Word* args, Word& result, int /*message*/, Word& /*local*/, Supplier s)
{
  const SEQ& seq1 = *static_cast<SEQ*>(args[0].addr);
  const SEQ& seq2 = *static_cast<SEQ*>(args[1].addr);
  const CcReal &cc_epsilon = *static_cast<CcReal*>(args[2].addr);
  const CcInt *cc_delta =
      HAS_DELTA ? static_cast<CcInt*>(args[3].addr) : nullptr;
  result = qp->ResultStorage(s);    // CcReal
  CcReal& sim = *static_cast<CcReal*>(result.addr);

/*
Require defined and non-empty sequences, a defined $epsilon$ and a defined
$delta$, if specified.

*/
  if (seq1.size() == 0 || seq2.size() == 0 || !cc_epsilon.IsDefined() ||
      (HAS_DELTA && !cc_delta->IsDefined())) {
    sim.SetDefined(false);
    return 0;
  }

/*
Require a positive $epsilon$.

*/
  const double epsilon = cc_epsilon.GetValue();
  if (epsilon <= 0.0) {
    sim.SetDefined(false);
    return 0;
  }

/*
Require a non-negative $delta$, if specified.

*/
  const int delta = HAS_DELTA ? cc_delta->GetValue() : -1;
  if (HAS_DELTA && delta < 0) {
    sim.SetDefined(false);
    return 0;
  }

  sim.Set(FUNC(seq1, seq2, epsilon, delta));
  return 0;
}

void TrajectorySimilarityAlgebra::addLCSSOp()
{
  AddOperator(
      LCSSInfo(), lcss_functions,
      LCSSSelect, LCSSTypeMap);
  AddOperator(
      RelLCSSInfo(), rel_lcss_functions,
      LCSSOpSelect, LCSSOpTypeMap);
  AddOperator(
      DistLCSSInfo(), dist_lcss_functions,
      LCSSOpSelect, LCSSOpTypeMap);
}

} //-- namespace tsa

/*
\begin{thebibliography}{ABCD99}

//[Oe] [\"{O}]

\bibitem[Aho90]{Aho90}
Alfred V.\ Aho.
\newblock {Algorithms for Finding Patterns in Strings}.
\newblock In {\em {Handbook of Theoretical Computer Science, Volume {A:}
  Algorithms and Complexity {(A)}}}, pages 255--300. 1990.

\bibitem[C[Oe]O05]{COO05}
Lei Chen, M.\ Tamer [Oe]zsu, and Vincent Oria.
\newblock {Robust and Fast Similarity Search for Moving Object Trajectories}.
\newblock In Fatma [Oe]zcan, editor, {\em {Proceedings of the {ACM} {SIGMOD}
  International Conference on Management of Data, Baltimore, Maryland, USA,
  June 14-16, 2005}}, pages 491--502. {ACM}, 2005,
  http://doi.acm.org/10.1145/1066157.1066213;
  http://dblp.uni-trier.de/rec/bib/conf/sigmod/ChenOO05.

\bibitem[VGK02]{VGK02}
Michail Vlachos, Dimitrios Gunopulos, and George Kollios.
\newblock {Discovering Similar Multidimensional Trajectories}.
\newblock In Rakesh Agrawal and Klaus R.\ Dittrich, editors, {\em {Proceedings
  of the 18th International Conference on Data Engineering, San Jose, CA, USA,
  February 26 - March 1, 2002}}, pages 673--684. {IEEE} Computer Society, 2002,
  http://dx.doi.org/10.1109/ICDE.2002.994784;
  http://dblp.uni-trier.de/rec/bib/conf/icde/VlachosGK02.


\end{thebibliography}

*/
