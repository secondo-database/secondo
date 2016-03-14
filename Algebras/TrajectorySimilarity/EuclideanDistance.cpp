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

[1] Implementation of the Euclidean Distance Operator

[2] EuclideanDistance.cpp

\tableofcontents

\noindent

1 Introduction

This file implements and registers the Euclidean distance operator for point
sequences as described in \cite[section 2]{COO05}.


1.1 Operator ~dist\_euclidean~

The operator

        $dist\_euclidean : SEQ \times SEQ\ [\times geoid] \rightarrow real$

with $SEQ \in \{pointseq,\ tpointseq\}$ determines the Euclidean distance
$D_{Eu}$ between two point sequences $P,\ Q$ of type ~SEQ~ that have the same
number $n>0$ of points.

The Euclidean distance of two point sequences is defined as

$$D_{Eu}(P,\thinspace Q) :=
  \sqrt{\sum_{i=1}^{n}d^{2}(p_{i},\thinspace q_{i})},$$

where $d^{2} : point \times point \rightarrow real$ is the squared Euclidean
distance of two points, defined as

$$d^{2}(p,\thinspace q) := (p_{x}-q_{x})^{2}+(p_{y}-q_{y})^{2}.$$

The temporal components of ~tpointseq~ objects are ignored. The computation
takes the ~geoid~ into account, if specified. If any of the two sequences is
~undefined~ or empty (i.e. it contains no point) or if the sequences have
different numbers of points, the result is ~undefined~.


2 Includes

*/

#include "PointSeq.h"
#include "TrajectorySimilarity.h"
#include "VectorTypeMapUtils.h"

#include "Geoid.h"


namespace tsa {

/*
3 Registration of Operators

3.1 ~dist\_euclidean~

This function expects two non-empty sequences with the same number of points.

*/
template<class SEQ>
double dist_euclidean(const SEQ& seq1, const SEQ& seq2, const Geoid* geoid)
{
  double dist = 0.0;
  for (size_t i = 0; i < seq1.GetNoComponents(); ++i) {
    const Point p1 = seq1.get(i);
    const Point p2 = seq2.get(i);
    dist += sqrEuclideanDistance(p1, p2, geoid);
  }
  return sqrt(dist);
}

template<class SEQ, bool HAS_GEOID>
int EuclideanDistValueMap(
    Word* args, Word& result, int /*message*/, Word& /*local*/, Supplier s)
{
  const SEQ& seq1 = *static_cast<SEQ*>(args[0].addr);
  const SEQ& seq2 = *static_cast<SEQ*>(args[1].addr);
  const Geoid* geoid = HAS_GEOID ? static_cast<Geoid*>(args[2].addr) : nullptr;
  result = qp->ResultStorage(s);    // CcReal
  CcReal& dist = *static_cast<CcReal*>(result.addr);

/*
Require defined and non-empty sequences of equal length.

*/
  if (seq1.GetNoComponents() != seq2.GetNoComponents() ||
      seq1.GetNoComponents() == 0) {
    dist.SetDefined(false);
    return 0;
  }

  dist.Set(dist_euclidean(seq1, seq2, geoid));
  return 0;
}

ValueMapping dist_euclidean_functions[] = {
  EuclideanDistValueMap<PointSeq,  /*HAS_GEOID*/ false>,
  EuclideanDistValueMap<PointSeq,  /*HAS_GEOID*/ true>,
  EuclideanDistValueMap<TPointSeq, /*HAS_GEOID*/ false>,
  EuclideanDistValueMap<TPointSeq, /*HAS_GEOID*/ true>,
  nullptr
};

struct DistEuclideanInfo : OperatorInfo
{
  DistEuclideanInfo() : OperatorInfo()
  {
    name      = "dist_euclidean";
    signature = PointSeq::BasicType() + " x " + PointSeq::BasicType()
                + " -> " + CcReal::BasicType();
    appendSignature(
                PointSeq::BasicType() + " x " + PointSeq::BasicType() + " x "
                + Geoid::BasicType() + " -> " + CcReal::BasicType());
    appendSignature(
                TPointSeq::BasicType() + " x " + TPointSeq::BasicType()
                + " -> " + CcReal::BasicType());
    appendSignature(
                TPointSeq::BasicType() + " x " + TPointSeq::BasicType() + " x "
                + Geoid::BasicType() + " -> " + CcReal::BasicType());
    syntax    = "dist_euclidean(_, _[, _])";
    meaning   = "Euclidean distance of two point sequences, that is the square "
                "root of the sum of squared Euclidean distances between "
                "corresponding points of the sequences. The geoid is taken "
                "into account, if specified. If any of the sequences is not "
                "defined or empty or if the sequences have different numbers "
                "of points, the result is undefined.\n"
                "The time complexity is O(n).";
  }
};

const mappings::VectorTypeMaps dist_euclidean_maps = {
  /*0*/ {{PointSeq::BasicType(), PointSeq::BasicType()},
    /* -> */ {CcReal::BasicType()}},
  /*1*/ {{PointSeq::BasicType(), PointSeq::BasicType(), Geoid::BasicType()},
    /* -> */ {CcReal::BasicType()}},
  /*2*/ {{TPointSeq::BasicType(), TPointSeq::BasicType()},
    /* -> */ {CcReal::BasicType()}},
  /*3*/ {{TPointSeq::BasicType(), TPointSeq::BasicType(), Geoid::BasicType()},
    /* -> */ {CcReal::BasicType()}}
};

ListExpr DistEuclideanTypeMap(ListExpr args)
{ return mappings::vectorTypeMap(dist_euclidean_maps, args); }

int DistEuclideanSelect(ListExpr args)
{ return mappings::vectorSelect(dist_euclidean_maps, args); }

void TrajectorySimilarityAlgebra::addEuclideanDistOp()
{
  AddOperator(
      DistEuclideanInfo(), dist_euclidean_functions,
      DistEuclideanSelect, DistEuclideanTypeMap);
}

} //-- namespace tsa

/*
\begin{thebibliography}{ABCD99}

//[Oe] [\"{O}]

\bibitem[C[Oe]O05]{COO05}
Lei Chen, M.\ Tamer [Oe]zsu, and Vincent Oria.
\newblock {Robust and Fast Similarity Search for Moving Object Trajectories}.
\newblock In Fatma [Oe]zcan, editor, {\em {Proceedings of the {ACM} {SIGMOD}
  International Conference on Management of Data, Baltimore, Maryland, USA,
  June 14-16, 2005}}, pages 491--502. {ACM}, 2005,
  http://doi.acm.org/10.1145/1066157.1066213;
  http://dblp.uni-trier.de/rec/bib/conf/sigmod/ChenOO05.

\end{thebibliography}

*/
