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
$P'= \langle p'_0,\ \dots,\ p'_{m-1}\rangle$ and
$Q'= \langle q'_0,\ \dots,\ q'_{n-1}\rangle$ match, if they have the same number
of items $m=n$ and for each $i \in \lbrack0,\ n \lbrack$ the item $p'_i$ matches
$q'_i$ within some neighbourhood.

For point sequences, \cite[section 2]{COO05} defines that two points $p, q$
match, if their maximum norm ($L^{\infty}$) distance is less than or equal to
$\epsilon > 0$, that is

        $(|p.x-q.x| \le \epsilon) \land (|p.y-q.y| \le \epsilon).$

In addition to the spatial criterion, \cite{VGK02} requires that the index
positions $i, j$ of the points $p, q$ in their original sequences $P, Q$ are at
most $\delta \ge 0$ places apart, that is $|i-j| \le \delta$.


1.1 Operators

The operators

        $lcss : SEQ \times SEQ \times real\ [\times\ int] \rightarrow int$,
        \newline
        $rel\_lcss : SEQ \times SEQ \times real\ [\times\ int] \rightarrow
        real$, and \newline
        $dist\_lcss : SEQ \times SEQ \times real\ [\times\ int] \rightarrow
        real$

with $SEQ \in \{pointseq,\ tpointseq\}$ and their overloads

        $lcss : mlabel \times mlabel\ [\times\ int] \rightarrow int$,
        \newline
        $rel\_lcss : mlabel \times mlabel\ [\times\ int] \rightarrow real$,
        and \newline
        $dist\_lcss : mlabel \times mlabel\ [\times\ int] \rightarrow real$

measure the similarity or the distance of two point sequences or ~mlabel~
objects based on the length of their Longest Common Subsequence.

The temporal components of a ~tpointseq~ or ~mlabel~ are ignored.

The point sequence overloads

        $\circ : SEQ \times SEQ \times real\ \rightarrow RESULT$

with $\circ(P,\ Q,\ \epsilon)$ consider two points matching, if their maximum
norm distance ($L^{\infty}$) is less than $\epsilon$, as described in
\cite[section 2]{COO05}. The point sequence overloads

        $\circ : SEQ \times SEQ \times real \times int\ \rightarrow RESULT$

with $\circ(P,\ Q,\ \epsilon,\ \delta)$ consider two points matching, if their
maximum norm distance ($L^{\infty}$) is less than $\epsilon$ and their index
positions in the original sequences are at most $\delta$ places apart, as
described in \cite{VGK02}.

Similarly, the ~mlabel~ overloads

        $\circ : mlabel \times mlabel\ \rightarrow RESULT$

with $\circ(P,\ Q)$ consider two labels matching, if they are equal. The
~mlabel~ overloads

        $\circ : mlabel \times mlabel \times int\ \rightarrow RESULT$

with $\circ(P,\ Q,\ \delta)$ consider two labels matching, if they are equal and
their index positions in the original ~mlabel~s are at most $\delta$ places
apart.

1.1.1 ~lcss~

The operator ~lcss~ determines the absolute length of the Longest Common
Subsequence of two sequences. The result is in the range $[0,\ \min(m,\ n)]$,
where $m,\ n$ are the lengths of the input sequences and a greater result value
indicates higher similarity. If any of the two sequences is ~undefined~, the
result is ~undefined~.

1.1.2 ~rel\_lcss~

The operator ~rel\_lcss~ determines the relative length of the Longest Common
Subsequence of two sequences, that is the absolute length of the LCSS divided by
the length of the shorter of the two sequences. \cite[def.\ 2]{VGK02} presents
this as the similarity function $S1$. The result is in the range $[0,\ 1]$,
where a value of 0 indicates low similarity and a value of 1 indicates that the
shorter sequence is a subsequence of the longer sequence. If any of the two
sequences is ~undefined~ or empty (i.e. it contains no point or label), the
result is ~undefined~.

1.1.3 ~dist\_lcss~

The operator ~dist\_lcss~ measures the distance of two  sequences in the range
$[0,\ 1]$. It is defined as $dist\_lcss = 1 - rel\_lcss$. \cite[def.\ 4]{VGK02}
presents this as the distance function $D1$. If any of the two sequences is
~undefined~ or empty (i.e. it contains no point or label), the result is
~undefined~.


2 Includes

*/

#include "EditDistanceAlgorithm.h"
#include "PointSeq.h"
#include "TrajectorySimilarity.h"
#include "VectorTypeMapUtils.h"

#include "../SymbolicTrajectory/Algorithms.h"   // stj::MLabel


namespace tsa {

/*
3 Helper Functions

Function template that tests if the items at positions $i1, i2$ of sequences
$seq1, seq2$, respectively, match. The default implementation for point
sequences tests if the spatial maximum norm distance ($L^{\infty}$) of the
points is less than $epsilon$.

*/
template<class SEQ>
inline static bool items_match(
    const SEQ& seq1, const SEQ& seq2,
    const size_t i1, const size_t i2, const double epsilon)
{
  const Point p1 = seq1.get(i1);
  const Point p2 = seq2.get(i2);
  return maxDistance(p1, p2) <= epsilon;
}

/*
Template specialization for ~stj::MLabel~ objects. It tests for equality of the
two labels.

*/
template<>
inline bool items_match(
    const stj::MLabel& seq1, const stj::MLabel& seq2,
    const size_t i1, const size_t i2, const double /*epsilon*/)
{
  stj::Label l1(/*defined*/ false);
  stj::Label l2(/*defined*/ false);
  seq1.GetBasic(i1, l1);
  seq2.GetBasic(i2, l2);
  return (l1 == l2);
}


/*
4 Registration of Operators

4.1 Common Declarations

Template for the type of the operator function with the following template
parameters:

  * ~SEQ~ is the sequence type of the operator, one of ~tsa::PointSeq~,
    ~tsa::TPointSeq~, and ~stj::MLabel~.

  * ~RES~ is the return type of the operator function. It is ~unsigned int~ for
    the basic ~lcss~ operator and ~double~ for the operators ~rel\_lcss~ and
    ~dist\_lcss~.

*/
template<class SEQ, typename RES>
using op_func_t = RES (*)(
    const SEQ&, const SEQ&, const double epsilon, const int delta);

/*
Struct template with specializations to determine the return type of the
operator function from SECONDO's type ~CCRES~.

*/
template<class CCRES> struct OpTypeHelper { };
template<> struct OpTypeHelper<CcInt> { using res_t = unsigned int; };
template<> struct OpTypeHelper<CcReal> { using res_t = double; };

/*
Template for value mapping function for LCSS-based operators with the following
template parameters:

  * ~SEQ~ is the sequence type of the operator, one of ~tsa::PointSeq~,
    ~tsa::TPointSeq~, and ~stj::MLabel~.

  * ~CCRES~ is the SECONDO return type of the value mapping function. It is
    ~CcInt~ for the basic ~lcss~ operator and ~CcReal~ for the operators
    ~rel\_lcss~ and ~dist\_lcss~.

  * ~NON\_EMPTY~ indicates whether the operator requires non-empty sequences.

  * ~HAS\_EPS~ indicates whether the overload has an $epsilon$ parameter.

  * ~HAS\_DEL~ indicates whether the overload has a $delta$ parameter.

  * ~FUNC~ is the operator function that performs the actual calculation.

*/
template<
    class SEQ, class CCRES, bool NON_EMPTY, bool HAS_EPS, bool HAS_DEL,
    op_func_t<SEQ, typename OpTypeHelper<CCRES>::res_t> FUNC>
int LCSSOpValueMap(
    Word* args, Word& result, int /*message*/, Word& /*local*/, Supplier s);


/*
4.2 ~lcss~

This operator function expects two defined sequences, an $epsilon > 0$ for point
sequences, and optionally a $delta \ge 0$.

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
      seq1.GetNoComponents(), seq2.GetNoComponents(),
      [&](const unsigned int i1, const unsigned int i2) -> bool {
/*
It would be a bug in the algorithm's implementation, is one of the indices was
out of range.

*/
        assert(
            i1 < static_cast<unsigned int>(seq1.GetNoComponents()) &&
            i2 < static_cast<unsigned int>(seq2.GetNoComponents()));

/*
If $delta \ne -1$, check for maximum distance of items in original
sequences.

*/
        if (delta != -1 && abs(i1 - i2) > delta)
          return false;

/*
Check for maximum spatial distance of points or equal labels.

*/
        return items_match(seq1, seq2, i1, i2, epsilon);
      }
  );

/*
Derive the LCSS length from the Edit Distance.

*/
  const unsigned int max_edits =
      seq1.GetNoComponents() + seq2.GetNoComponents();
  return (max_edits - ed) / 2;
}

ValueMapping lcss_functions[] = {
  //             SEQ,         CCRES, NON_EMPTY, HAS_EPS, HAS_DEL, FUNC
  LCSSOpValueMap<PointSeq,    CcInt, false,     true,    false,   lcss>,
  LCSSOpValueMap<PointSeq,    CcInt, false,     true,    true,    lcss>,
  LCSSOpValueMap<TPointSeq,   CcInt, false,     true,    false,   lcss>,
  LCSSOpValueMap<TPointSeq,   CcInt, false,     true,    true,    lcss>,
  LCSSOpValueMap<stj::MLabel, CcInt, false,     false,   false,   lcss>,
  LCSSOpValueMap<stj::MLabel, CcInt, false,     false,   true,    lcss>,
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
    appendSignature(
                stj::MLabel::BasicType() + " x " + stj::MLabel::BasicType()
                + " -> " + CcInt::BasicType());
    appendSignature(
                stj::MLabel::BasicType() + " x " + stj::MLabel::BasicType()
                + " x " + CcInt::BasicType() + " -> " + CcInt::BasicType());
    syntax    = "lcss(_, _[, _[, _]])";
    meaning   = "Length of Longest Common Subsequence (LCSS) of two point "
                "sequences or mlabel objects of lengths m, n. The result is in "
                "the range [0, min(m,n)], where a greater value indicates "
                "higher similarity. If any of the two sequences is undefined, "
                "the result is undefined.\n"
                "The point sequence overloads lcss(P, Q, epsilon) consider "
                "two points matching, if they are within spatial maximum norm "
                "distance epsilon.\n"
                "The point sequence overloads lcss(P, Q, epsilon, delta) "
                "additionally require that the indices of the two points in "
                "the original sequences P, Q are at most delta places apart.\n"
                "The mpoint oberload lcss(P, Q) considers two labels matching, "
                "if they are equal, and the mpoint overload lcss(P, Q, delta) "
                "additionally requires that the indices of the two labels in "
                "the original sequences P, Q are at most delta places apart.\n"
                "The time complexity is O((m+n)d) and the space complexity is "
                "O(max(m,n)), where d is the length of the LCSS.";
  }
};


/*
4.3 ~rel\_lcss~

This operator function expects two defined and non-empty sequences, an
$epsilon > 0$ for point sequences, and optionally a $delta \ge 0$.

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
  const unsigned int min_length =
      std::min(seq1.GetNoComponents(), seq2.GetNoComponents());

/*
The absolute LCSS length is at most the minimum sequence length. Therefore the
return value is in the range $[0, 1]$.

*/
  return static_cast<double>(l) / static_cast<double>(min_length);
}

ValueMapping rel_lcss_functions[] = {
  //             SEQ,         CCRES,  NON_EMPTY, HAS_EPS, HAS_DEL, FUNC
  LCSSOpValueMap<PointSeq,    CcReal, true,      true,    false,   rel_lcss>,
  LCSSOpValueMap<PointSeq,    CcReal, true,      true,    true,    rel_lcss>,
  LCSSOpValueMap<TPointSeq,   CcReal, true,      true,    false,   rel_lcss>,
  LCSSOpValueMap<TPointSeq,   CcReal, true,      true,    true,    rel_lcss>,
  LCSSOpValueMap<stj::MLabel, CcReal, true,      false,   false,   rel_lcss>,
  LCSSOpValueMap<stj::MLabel, CcReal, true,      false,   true,    rel_lcss>,
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
    appendSignature(
                stj::MLabel::BasicType() + " x " + stj::MLabel::BasicType()
                + " -> " + CcReal::BasicType());
    appendSignature(
                stj::MLabel::BasicType() + " x " + stj::MLabel::BasicType()
                + " x " + CcInt::BasicType() + " -> " + CcReal::BasicType());
    syntax    = "rel_lcss(_, _, _[, _])";
    meaning   = "Relative length of Longest Common Subsequence (LCSS) of two "
                "point sequences or mlabel objects of lengths m, n. The result "
                "is in the range [0, 1], where a value of 0 indicates low "
                "similarity and a value of 1 indicates that the shorter "
                "sequence is a subsequence of the longer sequence. If any of "
                "the two sequences is undefined or empty, the result is "
                "undefined.\n"
                "The same rules apply for matching of points and labels as for "
                "the lcss operator.\n"
                "The time complexity is O((m+n)d) and the space complexity is "
                "O(max(m,n)), where d is the length of the LCSS.";
  }
};


/*
4.4 ~dist\_lcss~

This operator function expects two defined and non-empty sequences, an
$epsilon > 0$ for point sequences, and optionally a $delta \ge 0$.

*/
template<class SEQ>
double dist_lcss(
    const SEQ& seq1, const SEQ& seq2,
    const double epsilon, const int delta = -1)
{
  return 1 - rel_lcss(seq1, seq2, epsilon, delta);
}

ValueMapping dist_lcss_functions[] = {
  //             SEQ,         CCRES,  NON_EMPTY, HAS_EPS, HAS_DEL, FUNC
  LCSSOpValueMap<PointSeq,    CcReal, true,      true,    false,   dist_lcss>,
  LCSSOpValueMap<PointSeq,    CcReal, true,      true,    true,    dist_lcss>,
  LCSSOpValueMap<TPointSeq,   CcReal, true,      true,    false,   dist_lcss>,
  LCSSOpValueMap<TPointSeq,   CcReal, true,      true,    true,    dist_lcss>,
  LCSSOpValueMap<stj::MLabel, CcReal, true,      false,   false,   dist_lcss>,
  LCSSOpValueMap<stj::MLabel, CcReal, true,      false,   true,    dist_lcss>,
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
    appendSignature(
                stj::MLabel::BasicType() + " x " + stj::MLabel::BasicType()
                + " -> " + CcReal::BasicType());
    appendSignature(
                stj::MLabel::BasicType() + " x " + stj::MLabel::BasicType()
                + " x " + CcInt::BasicType() + " -> " + CcReal::BasicType());
    syntax    = "dist_lcss(_, _, _[, _])";
    meaning   = "Distance of two point sequences or mlabel objects defined as "
                "(1 - rel_lcss). The result is in the range [0, 1], where a "
                "value of 0 indicates that the shorter sequence is a "
                "subsequence of the longer sequence and a value of 1 indicates "
                "low similarity. If any of the two sequences is undefined or "
                "empty, the result is undefined.\n"
                "The same rules apply for matching of points and labels as for "
                "the lcss operator.\n"
                "The time complexity is O((m+n)d) and the space complexity is "
                "O(max(m,n)), where d is the length of the LCSS.";
  }
};


/*
4.5 Common Implementation

The type maps of the operators differ only in the result type.

*/
template<class CCRES> struct OpMaps
{ static const mappings::VectorTypeMaps maps; };

template<class CCRES>
const mappings::VectorTypeMaps OpMaps<CCRES>::maps = {
  /*0*/ {{PointSeq::BasicType(), PointSeq::BasicType(), CcReal::BasicType()},
    /* -> */ {CCRES::BasicType()}},
  /*1*/ {{PointSeq::BasicType(), PointSeq::BasicType(), CcReal::BasicType(),
    CcInt::BasicType()}, /* -> */ {CCRES::BasicType()}},
  /*2*/ {{TPointSeq::BasicType(), TPointSeq::BasicType(), CcReal::BasicType()},
    /* -> */ {CCRES::BasicType()}},
  /*3*/ {{TPointSeq::BasicType(), TPointSeq::BasicType(), CcReal::BasicType(),
    CcInt::BasicType()}, /* -> */ {CCRES::BasicType()}},
  /*4*/ {{stj::MLabel::BasicType(), stj::MLabel::BasicType()},
    /* -> */ {CCRES::BasicType()}},
  /*5*/ {{stj::MLabel::BasicType(), stj::MLabel::BasicType(),
    CcInt::BasicType()}, /* -> */ {CCRES::BasicType()}}
};

template<class CCRES>
ListExpr LCSSOpTypeMap(ListExpr args)
{ return mappings::vectorTypeMap(OpMaps<CCRES>::maps, args); }

template<class CCRES>
int LCSSOpSelect(ListExpr args)
{ return mappings::vectorSelect(OpMaps<CCRES>::maps, args); }

/*
Template for value mapping function for LCSS-based operators. See above for a
description of the template parameters.

*/
template<
    class SEQ, class CCRES, bool NON_EMPTY, bool HAS_EPS, bool HAS_DEL,
    op_func_t<SEQ, typename OpTypeHelper<CCRES>::res_t> FUNC>
int LCSSOpValueMap(
    Word* args, Word& result, int /*message*/, Word& /*local*/, Supplier s)
{
  const SEQ& seq1 = *static_cast<SEQ*>(args[0].addr);
  const SEQ& seq2 = *static_cast<SEQ*>(args[1].addr);
  size_t argc = 2;
  const CcReal *cc_epsilon =
      HAS_EPS ? static_cast<CcReal*>(args[argc++].addr) : nullptr;
  const CcInt *cc_delta =
      HAS_DEL ? static_cast<CcInt*>(args[argc++].addr) : nullptr;
  result = qp->ResultStorage(s);    // CCRES
  CCRES& res = *static_cast<CCRES*>(result.addr);

/*
Require defined sequences, a defined $epsilon$, if specified, and a defined
$delta$, if specified.

*/
  if (!seq1.IsDefined() || !seq2.IsDefined() ||
      (HAS_EPS && !cc_epsilon->IsDefined()) ||
      (HAS_DEL && !cc_delta->IsDefined()))
  {
    res.SetDefined(false);
    return 0;
  }

/*
Require non-empty sequences, if specified.

*/
  if (NON_EMPTY && (seq1.GetNoComponents() == 0 || seq2.GetNoComponents() == 0))
  {
    res.SetDefined(false);
    return 0;
  }

/*
Require a positive $epsilon$, if specified.

*/
  const double epsilon = HAS_EPS ? cc_epsilon->GetValue() : NAN;
  if (HAS_EPS && epsilon < 0.0) {
    res.SetDefined(false);
    return 0;
  }

/*
Require a non-negative $delta$, if specified.

*/
  const int delta = HAS_DEL ? cc_delta->GetValue() : -1;
  if (HAS_DEL && delta < 0) {
    res.SetDefined(false);
    return 0;
  }

  res.Set(FUNC(seq1, seq2, epsilon, delta));
  return 0;
}

void TrajectorySimilarityAlgebra::addLCSSOp()
{
  AddOperator(
      LCSSInfo(), lcss_functions,
      LCSSOpSelect<CcInt>, LCSSOpTypeMap<CcInt>);
  AddOperator(
      RelLCSSInfo(), rel_lcss_functions,
      LCSSOpSelect<CcReal>, LCSSOpTypeMap<CcReal>);
  AddOperator(
      DistLCSSInfo(), dist_lcss_functions,
      LCSSOpSelect<CcReal>, LCSSOpTypeMap<CcReal>);
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
