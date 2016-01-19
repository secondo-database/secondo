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

[1] Implementation of the Locality In-between Polylines (LIP) Operator

\tableofcontents

\noindent

1 Introduction

This file implements and registers the Locality In-between Polylines distance
operator for point sequences as defined in \cite[def.\ 2]{PKM+07}.

The basic idea of the LIP operator is to measure the area of the shape formed by
the two sequences, if their first points are connected and their last points are
connected to close the shape.

On a more detailed level, the LIP distance $D_{LIP}(P,\ Q)$ is defined as the
sum of the weighted areas of the polygons defined by the intersection points of
the two sequences $P,\ Q$.

$$D_{LIP}(P,\ Q) = \sum_{\forall\ polygon} area(polygon) \cdot weight(polygon)$$

The weight of a polygon is defined as the length of the portions of the
sequences $P,\ Q$ that participate in the construction of the polygon divided by
the total length of the sequences.

$$weight(polygon) =
  \frac{length(Q_{polygon}) + length(P_{polygon})}{length(Q) + length(P)}$$

As \cite{PKM+07} points out, the LIP measure works correctly for point sequences
that follow a stable trend with no dramatic rotations. Below such pair of
sequences is called ~well-formed~. With other pairs of sequences the LIP
algorithm may yield self-intersecting polygons that can render the distance
measure meaningless or indefinite. The authors propose a separate algorithm
called ~GenLIP~, that searches a pair of sequences for bad segments and feeds
the good parts into the LIP algorithm. The ~GenLIP~ algorithm is not implemented
here.

The implementation of the LIP operator makes use of the classes ~Point~,
~HalfSegment~, and ~Region~ of the SpatialAlgebra to find intersections and to
calculate the areas of polygons.


1.1 Operator ~dist\_lip~

The operator

        $dist\_lip : SEQ \times SEQ\ [\times geoid] \rightarrow real$

with $SEQ \in \{pointseq,\ tpointseq\}$ determines the Locality In-between
Polylines distance for a pair of ~well-formed~ point sequences $P,\ Q$ of type
~SEQ~. If the pair of sequences is not ~well-formed~, the result is indefinite.

The temporal components of ~tpointseq~ objects are ignored. The computation
takes the ~geoid~ into account, if specified and implemented in the functions of
the SpatialAlgebra used here. If any of the two sequences is ~undefined~ or
contains less than two points, the result is ~undefined~.

The worst-case time complexity of the operator in the present implementation is
$O(n^2)$ and the space complexity is $O(n)$, where $n$ is the total number of
points in the sequences. The time complexity can be improved to $O(n \log n)$
according to \cite{PKM+07}.


2 Includes

*/

#include "PointSeq.h"
#include "SegmentIt.h"
#include "TrajectorySimilarity.h"
#include "VectorTypeMapUtils.h"

#include "Geoid.h"
#include "RegionTools.h"     // reverseCycle, buildRegion
#include "SpatialAlgebra.h"  // Point, HalfSegment, Region


namespace tsa {

/*
3 Helpers

Struct representing a polygon together with the length of the portions of the
original sequences that participate in the construction of the polygon.

*/
struct Polygon
{
  Polygon(const Region &&region, const double length)
    : region(region), length(length) { }
  const Region region;
  const double length;
};


/*
Create a polygon from a list of points and add it to the list of polygons.

*/
static void append_polygon(
    std::list<Polygon> &polygons, const std::list<::Point> &points,
    const double length)
{
/*
Create a cycle of points.

*/
  std::vector<::Point> cycle;
/*
In the worst case, that cycle has one more point than the original list of
points, where the additional point is used to close the cycle.
~std::list::size()~ has $O(1)$ complexity in C++11.

*/
  cycle.reserve(points.size() + 1);

/*
Copy the points to the cycle, but skip duplicates.

*/
  for (const ::Point &pt : points) {
    if (cycle.size() != 0 && AlmostEqual(pt, cycle.back()))
      continue;
    cycle.push_back(pt);
  }

/*
Close the cycle, if needed.

*/
  if (cycle.size() > 1 && !AlmostEqual(cycle.front(), cycle.back()))
    cycle.push_back(cycle.front());

/*
Do not keep a polygon, whose area is apparently empty. The first and the last
point of the cycle are always the same. Therefore at least four points are
needed to make up a non-empty area.

*/
  if (cycle.size() < 4)
    return;

/*
Force clockwise order of cycle, so that it makes a face.

*/
  if (!getDir(cycle))
    reverseCycle(cycle);

/*
Create the polygon and append it to the list.

*/
  std::vector<std::vector<::Point>> cycles;
  cycles.push_back(std::move(cycle));
  std::unique_ptr<Region> polygon(buildRegion(cycles));

  if (polygon.get() != nullptr)
    polygons.push_back(Polygon(std::move(*polygon), length));
}


/*
4 Registration of Operators

4.1 ~dist\_lip~

This function expects two point sequences, each with at least two points
(i.\ e.\ one segment), and an optional geoid.

*/
template<class SEQ>
double dist_lip(const SEQ& seq1, const SEQ& seq2, const Geoid* geoid = nullptr)
{
/*
List of polygons.

*/
  std::list<Polygon> polygons;

/*
Total length of the two sequences.

*/
  double total_length = 0.0;


/*
Phase 1. Find intersections and create list of polygons.

*/
  {
/*
Get begin and (one past) end segment iterators for the sequences.

*/
    const SegmentIt<SEQ> sit1begin = SegmentIt<SEQ>::begin(seq1);
    const SegmentIt<SEQ> sit1end   = SegmentIt<SEQ>::end(seq1);
    const SegmentIt<SEQ> sit2begin = SegmentIt<SEQ>::begin(seq2);
    const SegmentIt<SEQ> sit2end   = SegmentIt<SEQ>::end(seq2);

/*
List of points defining the current polygon.

*/
    std::list<::Point> points;

/*
Helper storing the points of the current polygon on the second sequence.

*/
    std::list<::Point> points2;

/*
Length of the first and second sequence, respectively, on the current polygon.

*/
    double len1 = 0.0;
    double len2 = 0.0;

/*
The segment where the current polygon starts on the second sequence.

*/
    SegmentIt<SEQ> sit2mark = sit2begin;

/*
The most recent intersection; initially ~undefined~.

*/
    ::Point prev_cross(/*defined*/ false);

/*
Iterate the segments of the first sequence.

*/
    for (SegmentIt<SEQ> sit1 = sit1begin; sit1 != sit1end; ++sit1)
    {
      const ::Point &pt1 = sit1->GetDomPoint();
      if (points.size())
        len1 += points.back().Distance(pt1, geoid);
      points.push_back(pt1);

/*
Prepare iteration of the second sequence.

*/
      points2.clear();
      if (prev_cross.IsDefined())
        points2.push_front(prev_cross);
      len2 = 0.0;

/*
Iterate the segments of the second sequence starting after the most recent
intersection.

*/
      for (SegmentIt<SEQ> sit2 = sit2mark; sit2 != sit2end; ++sit2)
      {
/*
Note: Is appears to be expensive to build up $points2$ and $len2$ each time this
inner for loop is executed. Instead, this could be performed just once when an
intersection is found or the ends of both sequences are reached.

*/
        const ::Point &pt2 = sit2->GetDomPoint();
        if (points2.size())
          len2 += points2.front().Distance(pt2, geoid);
        points2.push_front(pt2);

/*
Check for an intersection of the current segments.

*/
        ::Point cross;
        if (sit1->Intersection(*sit2, cross, geoid)) {
/*
The current segments intersect. A polygon is defined by the concatenation of
$points$, $cross$, and $points2$.

Note: ~std::list::splice()~ clears $points2$.

*/
          len1 += points.back().Distance(cross, geoid);
          points.push_back(cross);
          len2 += points2.front().Distance(cross, geoid);
          points.splice(points.end(), points2);

/*
Append the polygon to the list.

*/
          const double len = len1 + len2;
          append_polygon(polygons, points, len);
          total_length += len;

/*
Start the next polygon.

*/
          prev_cross = cross;
          points.clear();
          points.push_back(prev_cross);
          points2.push_front(prev_cross);
          len1 = 0.0;
          len2 = 0.0;
          sit2mark = sit2 + 1;
          break;
        }
      }
    }

/*
The remaining points make up the last polygon. Also add the end points of the
last segments.

*/
    if (sit1begin != sit1end) {
      const ::Point &pt1 = (sit1end-1)->GetSecPoint();
      if (points.size())
        len1 += points.back().Distance(pt1, geoid);
      points.push_back(pt1);
    }
    if (sit2begin != sit2end) {
      const ::Point &pt2 = (sit2end-1)->GetSecPoint();
      if (points2.size())
        len2 += points2.front().Distance(pt2, geoid);
      points2.push_front(pt2);
    }
    points.splice(points.end(), points2);

/*
Append the last polygon to the list.

*/
    const double len = len1 + len2;
    append_polygon(polygons, points, len);
    total_length += len;
  }

/*
If the total length of the two sequences on the polygons is 0, we cannot weight
the areas of the polygons. Instead, we return a LIP distance of 0, assuming that
the polygon areas are also empty.

*/
  if (total_length == 0.0)
    return 0.0;


/*
Phase 2. Process polygons.

For each polygon determine its area and its weight and add the product of these
two to the LIP distance.

*/
  double dist = 0.0;
  for (const Polygon &polygon : polygons) {
    const double area = polygon.region.Area(geoid);
    const double weight = polygon.length / total_length;
    dist += area * weight;
  }

  return dist;
}

template<class SEQ, bool HAS_GEOID>
int LIPDistValueMap(
    Word* args, Word& result, int /*message*/, Word& /*local*/, Supplier s)
{
  const SEQ& seq1 = *static_cast<SEQ*>(args[0].addr);
  const SEQ& seq2 = *static_cast<SEQ*>(args[1].addr);
  const Geoid* geoid = HAS_GEOID ? static_cast<Geoid*>(args[2].addr) : nullptr;
  result = qp->ResultStorage(s);    // CcReal
  CcReal& dist = *static_cast<CcReal*>(result.addr);

/*
Require defined sequences with at least two points each.

*/
  if (seq1.GetNoComponents() < 2 || seq2.GetNoComponents() < 2) {
    dist.SetDefined(false);
    return 0;
  }

  dist.Set(dist_lip(seq1, seq2, geoid));
  return 0;
}

ValueMapping dist_lip_functions[] = {
  LIPDistValueMap<PointSeq,  /*HAS_GEOID*/ false>,
  LIPDistValueMap<PointSeq,  /*HAS_GEOID*/ true>,
  LIPDistValueMap<TPointSeq, /*HAS_GEOID*/ false>,
  LIPDistValueMap<TPointSeq, /*HAS_GEOID*/ true>,
  nullptr
};

struct DistLIPInfo : OperatorInfo
{
  DistLIPInfo() : OperatorInfo()
  {
    name      = "dist_lip";
    signature = PointSeq::BasicType() + " x " + PointSeq::BasicType()
                + " -> " + CcReal::BasicType();
    appendSignature(
                PointSeq::BasicType() + " x " + PointSeq::BasicType()
                + " x " + Geoid::BasicType() + " -> " + CcReal::BasicType());
    appendSignature(
                TPointSeq::BasicType() + " x " + TPointSeq::BasicType()
                + " -> " + CcReal::BasicType());
    appendSignature(
                TPointSeq::BasicType() + " x " + TPointSeq::BasicType()
                + " x " + Geoid::BasicType() + " -> " + CcReal::BasicType());
    syntax    = "dist_lip(_, _[, _])";
    meaning   = "Locality In-between Polylines (LIP) distance of a well-formed "
                "pair of point sequences. This is defined as the sum of the "
                "weighted areas of the polygons defined by the intersection "
                "points of the two sequences. If the pair of sequences is not "
                "well-formed, the result is indefinite.\n"
                "The geoid is taken into account, if specified and implemented "
                "in the functions of the SpatialAlgebra used here. If any of "
                "the sequences is undefined or has less than two points, the "
                "result is undefined.\n"
                "The worst-case time complexity of the operator in the present "
                "implementation is O(n^2) and the space complexity is O(n), "
                "where n is the total number of points in the sequences.";
  }
};

const mappings::VectorTypeMaps dist_lip_maps = {
  /*0*/ {{PointSeq::BasicType(), PointSeq::BasicType()},
    /* -> */ {CcReal::BasicType()}},
  /*1*/ {{PointSeq::BasicType(), PointSeq::BasicType(),
    Geoid::BasicType()}, /* -> */ {CcReal::BasicType()}},
  /*2*/ {{TPointSeq::BasicType(), TPointSeq::BasicType()},
    /* -> */ {CcReal::BasicType()}},
  /*3*/ {{TPointSeq::BasicType(), TPointSeq::BasicType(),
    Geoid::BasicType()}, /* -> */ {CcReal::BasicType()}}
};

ListExpr DistLIPTypeMap(ListExpr args)
{ return mappings::vectorTypeMap(dist_lip_maps, args); }

int DistLIPSelect(ListExpr args)
{ return mappings::vectorSelect(dist_lip_maps, args); }

void TrajectorySimilarityAlgebra::addLIPDistOp()
{
  AddOperator(
      DistLIPInfo(), dist_lip_functions,
      DistLIPSelect, DistLIPTypeMap);
}

} //-- namespace tsa

/*
\begin{thebibliography}{ABCD99}

\bibitem[PKM+07]{PKM+07}
Nikos Pelekis, Ioannis Kopanakis, Gerasimos Marketos, Irene Ntoutsi, Gennady L.
  \ Andrienko, and Yannis Theodoridis.
\newblock {Similarity Search in Trajectory Databases}.
\newblock In {\em {14th International Symposium on Temporal Representation and
  Reasoning {(TIME} 2007), 28-30 June 2007, Alicante, Spain}}, pages 129--140.
  {IEEE} Computer Society, 2007, http://dx.doi.org/10.1109/TIME.2007.59;
  http://dblp.uni-trier.de/rec/bib/conf/time/PelekisKMNAT07.

\end{thebibliography}

*/
