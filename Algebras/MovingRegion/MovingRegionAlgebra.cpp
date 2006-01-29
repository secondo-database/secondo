/*

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]
//[ue] [\"u]
//[ae] [\"a]
//[oe] [\"o]

[1] The ~MovingRegionAlgebra~

December 2005, initial version created by Holger M[ue]nx.

December 2005, Victor Almeida deleted the deprecated algebra levels
(~executable~, ~descriptive~, and ~hibrid~). Only the executable
level remains. Models are also removed from type constructors.

January 2006, various bugfixes and improvements by Holger M[ue]nx,
with the help of Victor Almeida and Thomas Behr.

[TOC]

1 Introduction

The ~MovingRegionAlgebra~ provides datatypes and operators relating to
moving regions.

To keep the ~TemporalAlgebra~ stable during development, the
~MovingRegionAlgebra~ has been developed as an ~independent~ algebra. Once
it is sufficiently stable, it is planned to merge its code into the
~TemporalAlgebra~. Due to the deadline of the bachelor's thesis and the
complexity of the ~MovingRegionAlgebra~, the current version should be
considered as prototype or proof of concept only, and needs to be finalised
before production usage.

The MovingRegionAlgebra has been developed for the bachelor thesis of
Holger M[ue]nx together with Prof. Dr. G[ue]ting, Fachbereich Informatik,
Fernuniversit[ae]t Hagen.

1 Known bugs, issues and missing features

Open:

  * Bug: List representation checks incorrect for
    ~(update mv := ((movingregion)((0.0 0.0 true true)(0.0 0.0 1.0 1.0))));~.
    Aleksej Struk found this issue.

    Update: Debug output at the beginning on ~InMRegion()~ indicated that
    this problem actually occurs before ~InMRegion()~ is called. It is very
    likely that this problem is not caused by this algebra.

  * Bug: Sorting units at the beginning of RefinementPartition() is missing.
    Constructor only works if units appear in ~mr~ and ~mp~ in proper order!

  * Bug: Calculations with values of datatype $Instant$ are done with
    double precision only and not with the datatypes own calculation
    operations.

  * Not confirmed: J[oe]rg Schmidt thinks there is an issue in the
    generation of the refinement partition if two intervals start or
    end at the same instant.

  * Feature: Debug bug output is very verbose. Due to its verbosity, it
    has impact on the algebra's performance, when enabled. It would be useful
    to have different debug levels.

Closed:

  * Bug: ~initial~ and ~final~ resulting in failed ~assert()~ when unit's
    interval is open in the respective instant.

    Resolved: Added the required checks.

  * Bug: Objects created in the server version of SECONDO are not
    compatible with the stand-alone version of SECONDO (and vice versa).

    Resolved: Replaced weird implementation of ~OpenMRegion()~ and
    ~SaveMRegion()~ with code based on functions ~Open()~ and ~Save()~ 
    from ~TupleElement~. Thomas Behr contributed this valuable hint!

  * Bug: ~URegion::AddSegment()~ checks for moving segment intersections
    outside current region unit, which is incorrect.

    Resolved: Offset ~segmentsStartPos~ in ~URegion::AddSegment()~ was
    missing.

  * Bug: Awkward memory leak at the end of ~URegion::TemporalFunction()~.

    Resolved: Freeing memory now. Resulting code may not be compatible
    with other compilers than gcc.

  * Bug: References to ~URegion~ and ~UPoint~ instead of ~Unit1~ and ~Unit2~
    in ~RefinementPartition()~. Victor Almeida found this one!

    Resolved: Replaced references.

  * Bug: ~intimeregion~ object creation via ~const intimeregion
    value ...~ results in a failed assertion.

    Resolved: Various modifications outside the MovingRegionAlgebra fixed
    this without any modifications required for the MovingRegionAlgebra
    itself.

  * Bug: Rounding issues have been observed and could be traced back to the
    value of constant ~eps~ in section \ref{defines}.

    Resolved: Implemented operator ~mraprec~ which allows to set precision
    without re-compiling the algreba.

  * Bug: ~MRegion~ objects cannot be imported into SECONDO according
    to Thomas Behr.

    Resolved: Implemented operator ~mraprec~ which allows to set precision
    without re-compiling the algreba.

*/

/*
1 Defines and includes
\label{defines}

*/

#include <queue>
#include <stdexcept>

#include "Algebra.h"
#include "NestedList.h"
#include "QueryProcessor.h"
#include "StandardTypes.h"

#include "TemporalAlgebra.h"

#include "DateTime.h"
using namespace datetime;

extern NestedList* nl;
extern QueryProcessor* qp;

/*
Set ~MRA\_DEBUG~ to ~true~ for debug output. Please note that debug output is
very verbose and has significant negative input on the algebra's performance.
Only enable debug output if you know what you are doing!

*/
const bool MRA_DEBUG = false;

/*
Two floating point numbers are considered equal if their difference is
less than the constant ~eps~.

Please note that issues have been observed with both too large and too small
values of ~eps~. Prof. Dr. G[ue]ting reported an example with incorrect
behaviour of operator ~at~ for the value $0.00001$. Thomas Behr reported that
correct moving regions have been rejected for value $0.00000001$.

*/
//const double eps = 0.0001;
//const double eps = 0.00000001;
double eps = 0.00001;

/*
1 Helper functions

1.1 Function ~simpleSelect()~

Simple selection function for non-overloaded operators.

*/
static int simpleSelect(ListExpr args) {
    return 0;
}

/*
1.1 Comparison functions for ~double~

To avoid rounding issues, comparisons for equality are done using constant
~eps~.

1.1.1 Function ~nearlyEqual()~

Returns ~true~ if $-eps \le a-b \le eps$.

*/
static bool nearlyEqual(double a, double b) {
    return abs(a-b) <= eps;
}

/*
1.1.1 Function ~lowerOrNearlyEqual()~

Returns ~true~ if $a \le b+eps$.

*/
static bool lowerOrNearlyEqual(double a, double b) {
    return a < b || nearlyEqual(a, b);
}

/*
1.1.1 Function ~lower()~

Returns ~true~ if $a < b-eps$.

*/
static bool lower(double a, double b) {
    return a < b-eps;
}

/*
1.1.1 Function ~greaterOrNearlyEqual()~

Returns ~true~ if $a \ge b-eps$.

*/
static bool greaterOrNearlyEqual(double a, double b) {
    return a > b || nearlyEqual(a, b);
}

/*
1.1.1 Function ~between()~

Returns ~true~ if $a-eps \le x \le b+eps$ or $b-eps \le x \le a+eps$.

*/
static bool between(double a, double x, double b) {
    return
        (lowerOrNearlyEqual(a, x) && lowerOrNearlyEqual(x, b))
        || (lowerOrNearlyEqual(b, x) && lowerOrNearlyEqual(x, a));
}

/*
1.1 Function ~minmax4()~

Returns the minimum and maximum value of $a$, $b$, $c$ and $d$ in $min$
and $max$.

*/
static void minmax4(double a,
                    double b,
                    double c,
                    double d,
                    double& min,
                    double& max) {
    if (MRA_DEBUG) cerr << "minmax4() called" << endl;

    min = a;
    max = a;

    if (b < min) min = b;
    if (b > max) max = b;
    if (c < min) min = c;
    if (c > max) max = c;
    if (d < min) min = d;
    if (d > max) max = d;
}

/*
1 Mathematical and geometrical functions

1.1 Function ~GaussTransform()~

Apply the Gauss transformation to system of equations. The left hand sides
of the equations are in matrix $a$ with $n$ rows and $m$ columns and the
right hand sides are in vector $b$ with $n$ rows.

The transformed matrix and vector are returned in $a$ and $b$.

*/
static void GaussTransform(const unsigned int n,
                           const unsigned int m,
                           double** a,
                           double* b) {
    if (MRA_DEBUG)
        cerr << "GaussTransform() called, n="
             << n << " m=" << m << endl;

    if (MRA_DEBUG)
        for (unsigned int j = 0; j < n; j++) {
            for (unsigned int k = 0; k < m; k++)
                fprintf(stderr, "%7.3f ", a[j][k]);
            fprintf(stderr, "| %7.3f\n", b[j]);
        }

/*
For each row...

*/
    for (unsigned int i = 0; i < n-1; i++) {
/*
Check for non-zero element in column $i$ below and including row $i$, which
can be used as pivot.

*/
        if (MRA_DEBUG) cerr << "i=" << i << endl;
        unsigned int j;
        for (j = i; j < n && a[j][i] == 0; j++)
            if (MRA_DEBUG)
                cerr << " failed as pivot: a["
                     << j
                     << "]["
                     << i
                     << "]="
                     << a[j][i]
                     << endl;

/*
If we found a pivot, apply it to the rows below row $i$. If we did not find
a pivot, column $i$ is already zero below and including row $i$ and we do
not have to do anything.

*/
        if (j != n) {
            if (MRA_DEBUG)
                cerr << " pivot: a["
                     << j
                     << "]["
                     << i
                     << "]="
                     << a[j][i]
                     << endl;

/*
The pivot is in row $j$. If $j\ne i$, the pivot is in another row than row $i$.
Swap rows $i$ and $j$ in this case.

*/
            if (j != i) {
                double dummy = b[i];
                b[i] = b[j];
                b[j] = dummy;
                for (unsigned int k = 0; k < m; k++) {
                    dummy = a[i][k];
                    a[i][k] = a[j][k];
                    a[j][k] = dummy;
                }
            }

/*
Subtract row $i$ from each row below row $i$, multiplied with factor $f$,
which is calculated so that it sets the element in column $i$ of each row
to $0$ during the substraction.

*/
            for (j = i+1; j < n; j++) {
                double f = a[j][i]/a[i][i];
                if (MRA_DEBUG) {
                    cerr << " j=" << j << endl;
                    cerr << "  f=" << f << endl;
                }
                a[j][i] = 0;
                for (unsigned int k = i+1; k < m; k++)
                    a[j][k] -= a[i][k]*f;
                b[j] -= b[i]*f;
            }
        }

        if (MRA_DEBUG)
            for (j = 0; j < n; j++) {
                for (unsigned int k = 0; k < m; k++)
                    fprintf(stderr, "%7.3f ", a[j][k]);
                fprintf(stderr, "| %7.3f\n", b[j]);
            }
    }
}

/*
1.1 Intersections between two segments

1.1.1 Function ~specialSegmentIntersects2()~

Returns ~true~ if the specified segment and line intersect
in three-dimensional space $(x, y, t)$.
It is not considered as intersection if they touch in their end points.

The segment connects the points $(l1p1x, l1p1y, z)$ and $(l1p2x, l1p2y, z)$,
ie.\ segment 1 is parallel to the $(x, y)$-plane.

The line is $P+Q\cdot t$. The line must not be parallel to the $(x, y)-plane$.

*/
static bool specialSegmentIntersects2(double z,
                                      double l1p1x,
                                      double l1p1y,
                                      double l1p2x,
                                      double l1p2y,
                                      double* P,
                                      double* Q) {
    if (MRA_DEBUG) {
        cerr << "specialSegmentIntersects2() called"
             << endl;
        cerr << "specialSegmentIntersects2() line 1: ("
             << l1p1x
             << ", "
             << l1p1y
             << ", "
             << z
             << ")-("
             << l1p2x
             << ", "
             << l1p2y
             << ", "
             << z
             << ")"
             << endl;
        cerr << "specialSegmentIntersects2() line 2: ("
             << P[0]
             << ", "
             << P[1]
             << ", "
             << P[2]
             << ")-("
             << Q[0]
             << ", "
             << Q[1]
             << ", "
             << Q[2]
             << ")*t"
             << endl;
    }

/*
Calculate point $(x, y)$ where the line intersects the plane parallel to the
$(x, y)$-plane with distance z from the $(x, y)$-plane.

*/
    double t2 = (z-P[2])/Q[2];

    if (MRA_DEBUG)
        cerr << "specialSegmentIntersects2() t="
             << t2
             << endl;

/*
If $t2<>z$, the segments do not intersect.

*/
    if (!nearlyEqual(t2, z)) {
        if (MRA_DEBUG)
            cerr
                << "specialSegmentIntersects2() no intersection #0"
                << endl;

        return false;
    }

    double x = P[0]+Q[0]*t2;
    double y = P[1]+Q[1]*t2;

    if (MRA_DEBUG)
        cerr << "specialSegmentIntersects2() x="
             << x
             << " y="
             << y
             << endl;

    double l1MinX = l1p1x < l1p2x ? l1p1x : l1p2x;
    double l1MaxX = l1p1x > l1p2x ? l1p1x : l1p2x;
    double l1MinY = l1p1y < l1p2y ? l1p1y : l1p2y;
    double l1MaxY = l1p1y > l1p2y ? l1p1y : l1p2y;

    if (MRA_DEBUG)
        cerr << "specialSegmentIntersects2() l1MinX="
             << l1MinX
             << " l1MaxX="
             << l1MaxX
             << " l1MinY="
             << l1MinY
             << " l1MaxY="
             << l1MaxY
             << endl;

    if (nearlyEqual(l1p1x, l1p2x) && nearlyEqual(l1p1y, l1p2y)) {
/*
The segment is actually a point.

*/
        if (MRA_DEBUG)
            cerr
                << "specialSegmentIntersects2() "
                << "no intersection #1" << endl;

        return false;
    } else if (nearlyEqual(l1p1x, l1p2x)) {
/*
The segment is vertical in the $(x, y)$-plane.

*/
        if (nearlyEqual(x, l1p1x)
            && lower(l1MinY, y)
            && lower(y, l1MaxY)) {
            if (MRA_DEBUG)
                cerr
                    << "specialSegmentIntersects2() "
                    << "intersects #2" << endl;

            return true;
        } else {
            if (MRA_DEBUG)
                cerr
                    << "specialSegmentIntersects2() "
                    << "no intersection #2" << endl;

            return false;
        }
    } else if (nearlyEqual(l1p1y, l1p2y)) {
/*
The segment is horizontal in the $(x, y)$-plane.

*/
        if (nearlyEqual(y, l1p1y)
            && lower(l1MinX, x)
            && lower(x, l1MaxX)) {
            if (MRA_DEBUG)
                cerr
                    << "specialSegmentIntersects2() "
                    << "intersects #3" << endl;

            return true;
        } else {
            if (MRA_DEBUG)
                cerr
                    << "specialSegmentIntersects2() "
                    << "no intersection #3" << endl;

            return false;
        }
    } else {
/*
Otherwise, $l1p1x\ne l1p2x$ and $l1p1y\ne l1p2y$ so that we can happily
use the quotients below.

First, check whether $(x, y)$ is on the line through the segment.

*/
        double t1 = (x-l1p1x)/(l1p2x-l1p1x);

        if (!nearlyEqual(t1, (y-l1p1y)/(l1p2y-l1p1y))) {
            if (MRA_DEBUG)
                cerr
                    << "specialSegmentIntersects2() "
                    << "no intersection #4a" << endl;

            return false;
        }

/*
Verify that $(x, y)$ is actually on the segment: As we already know that
it is on the line through the segment, we just have to check its bounding
box parallel to the $(x, y)$-plane.

*/
        if (lowerOrNearlyEqual(x, l1MinX)
            || lowerOrNearlyEqual(l1MaxX, x)
            || lowerOrNearlyEqual(y, l1MinY)
            || lowerOrNearlyEqual(l1MaxY, y)) {
            if (MRA_DEBUG)
                cerr << "specialSegmentIntersects2() "
                     << "no intersection #4b" << endl;

            return false;
        }

        if (MRA_DEBUG)
            cerr << "specialSegmentIntersects2() "
                 << "intersects" << endl;

        return true;
    }
}

/*
1.1.1 Function ~specialSegmentIntersects1()~ (with $z$)
\label{ssi}

Returns ~true~ if the two specified segments intersect
in three-dimensional space $(x, y, t)$.
It is not considered as intersection if they touch in their end points.

The two lines must meet the following conditions.

Segment 1 connects the points $(l1p1x, l1p1y, 0)$ and $(l1p2x, l1p2y, dt)$.

Segment 2 connects the points $(l2p1x, l2p1y, 0)$ and $(l2p2x, l2p2y, dt)$.

$z$ will contain the $z$-coordinate of the intersection point, if it
exists.

*/
static bool specialSegmentIntersects1(double dt,
                                      double l1p1x,
                                      double l1p1y,
                                      double l1p2x,
                                      double l1p2y,
                                      double l2p1x,
                                      double l2p1y,
                                      double l2p2x,
                                      double l2p2y,
                                      double& z) {
    if (MRA_DEBUG) {
        cerr << "specialSegmentIntersects1() w/ z component called"
             << endl;
        cerr << "specialSegmentIntersects1() line 1: ("
             << l1p1x
             << ", "
             << l1p1y
             << ", 0.0)-("
             << l1p2x
             << ", "
             << l1p2y
             << ", "
             << dt
             << ")"
             << endl;
        cerr << "specialSegmentIntersects1() line 2: ("
             << l2p1x
             << ", "
             << l2p1y
             << ", 0.0)-("
             << l2p2x
             << ", "
             << l2p2y
             << ", "
             << dt
             << ")"
             << endl;
    }

/*
Check if both segments are identical or touch in their endpoints.

*/
    if (nearlyEqual(l1p1x, l2p1x)
        && nearlyEqual(l1p1y, l2p1y)) {
        if (nearlyEqual(l1p2x, l2p2x)
            && nearlyEqual(l1p2y, l2p2y)) {
            if (MRA_DEBUG)
                cerr << "specialSegmentIntersects1() same segment"
                     << endl;

            return false;
        } else {
            if (MRA_DEBUG)
                cerr << "specialSegmentIntersects1() "
                     << "segments touch in ("
                     << l1p1x << ", " << l1p1y << ")"
                     << endl;
            return false;
        }
    } else if (nearlyEqual(l1p2x, l2p2x)
               && nearlyEqual(l1p2y, l2p2y)) {
            if (MRA_DEBUG)
                cerr << "specialSegmentIntersects1() "
                     << "segments touch in ("
                     << l1p2x << ", " << l1p2y << ")"
                     << endl;

        return false;
    }

/*
The line through segment 1 is
$(l1p1x, l1p1y, 0)+(l1p2x-l1p1x, l1p2y-l1p2y, dt)\cdot s$.
The line through segment 2 is
$(l2p1x, l2p1y, 0)+(l2p2x-l1p1x, l2p2y-l2p2y, dt)\cdot t$.

The intersection of the two lines is the solution of the following
linear system of equations:
\begin{eqnarray}
\begin{array}{ccc}
\begin{array}{cc}
l1p2x-l1p1x & l2p1x-l2p2x \\
l1p2y-l1p1y & l2p1y-l2p2y \\
dt          & -dt
\end{array} & \left| \begin{array}{c}
l2p1x-l1p1x \\
l2p1y-l1p1y \\
0
\end{array} \right.
\end{array}
\nonumber\end{eqnarray}

We put the left handed sides of the equations in matrix $a$ and the right
handed sides into array $b$ and are applying the Gaussian elimination to these:

*/
    double A[3][2] =
        {{ l1p2x-l1p1x, l2p1x-l2p2x },
         { l1p2y-l1p1y, l2p1y-l2p2y },
         { dt,          -dt         }};
    double B[3] =
        { l2p1x-l1p1x, l2p1y-l1p1y, 0 };

    double* Ap[] = { A[0], A[1], A[2] };

    GaussTransform(3, 2, Ap, B);

/*
Now, we will determine the solution $t$ from the transformed system.
We examine each row from the bottom to the top:

*/
    for (int i = 2; i >= 0; i--) {
        if (!nearlyEqual(Ap[i][0], 0.0)) break;

/*
Row is in format $0 c \left| b \right.$.

*/
        if (nearlyEqual(Ap[i][1], 0.0)) {
/*
Row is in format $0 0 \left| b \right.$. Check if there is a contradiction
indicating that there are no solutions.

*/
            if (nearlyEqual(B[i], 0.0))
                continue;
            else {
                if (MRA_DEBUG)
                    cerr << "specialSegmentIntersects1() "
                         << "do not intersect #1"
                         << endl;

                return false;
            }
        } else {
            z = dt*B[i]/Ap[i][1];

/*
Row is in format $0 c \left| b \right.$ with $c\ne 0$. All rows below are
entirely 0.
Inserting $t=b/c$ into the equation of line 2 yields the value $z=dt\cdot b/c$
for the third component. If $0\le z \le dt$, the two segments intersect. If
$z=0$ or $z=dt$, they touch at one of their end points, which is not considered
as intersection.

*/
            if (MRA_DEBUG)
                cerr << "specialSegmentIntersects1() z="
                     << z << endl;

            if (lowerOrNearlyEqual(0.0, z)
                && lowerOrNearlyEqual(z, dt)) {
                if (MRA_DEBUG)
                    cerr << "specialSegmentIntersects1() "
                         << "intersect"
                         << endl;

                return true;
            } else {
                if (MRA_DEBUG)
                    cerr << "specialSegmentIntersects1() "
                         << "do not intersect #2"
                         << endl;

                return false;
            }
        }
    }

/*
Both segments are located on the same line. We have to check whether they
are overlapping.

*/

    if ((between(l1p1x, l2p1x, l1p2x) 
         && between(l1p1y, l2p1y, l1p2y))
        || (between(l1p1x, l2p2x, l1p2x)
            && between(l1p1y, l2p2y, l1p2y))
        || (between(l2p1x, l1p1x, l2p2x) 
            && between(l2p1y, l1p1y, l2p2y))
        || (between(l2p1x, l1p2x, l2p2x)
            && between(l2p1y, l1p2y, l2p2y))) 
        return true;
    else
        return false;
}

/*
1.1.1 Function ~specialSegmentIntersects1()~ (without $z$)

Same as function in section \ref{ssi}, but not returning the $z$-coordinate
of the intersection point.

*/
static bool specialSegmentIntersects1(double dt,
                                      double l1p1x,
                                      double l1p1y,
                                      double l1p2x,
                                      double l1p2y,
                                      double l2p1x,
                                      double l2p1y,
                                      double l2p2x,
                                      double l2p2y) {
    if (MRA_DEBUG)
        cerr << "specialSegmentIntersects1() w/o z-component called"
             << endl;

    double dummy;

    return
        specialSegmentIntersects1(dt,
                                  l1p1x, l1p1y, l1p2x, l1p2y,
                                  l2p1x, l2p1y, l2p2x, l2p2y,
                                  dummy);
}

/*
1.1 Intersections between two trapeziums

1.1.1 Function ~specialTrapeziumIntersects()~ 
\label{stinu}

Returns ~true~ if and only if the two specified special trapeziums intersect
in three-dimensional space $(x, y, t)$.

The two trapeziums must meet the following conditions. These conditions are
not checked in the function. If the conditions are not met, the function will
deliver incorrect results.

Trapezium 1 is spanned by segments ~(t1p1x, t1p1y, 0)~ to ~(t1p2x, t1p2y, 0)~,
~(t1p1x, t1p1y, 0)~ to ~(t1p3x, t1p3y, dt)~, ~(t1p3x, t1p3y, dt)~ to
~(t1p4x, t1p4x, dt)~ and ~(t1p2x, t1p2y, 0)~ to ~(t1p4x, t1p4x, dt)~.
Allowed are either ~(t1p1x, t1p1y, 0)=(t1p2x, t1p2y, 0)~ or
~(t1p3x, t1p3y, dt)=(t1p4x, t1p4x, dt)~ but not both. Segments
~(t1p1x, t1p1y, 0)~ to ~(t1p2x, t1p2y, 0)~ and
~(t1p3x, t1p3y, dt)~ to ~(t1p4x, t1p4x, dt)~ are collinear.

Trapezium 2 is spanned by segments ~(t2p1x, t2p1y, 0)~ to ~(t2p2x, t2p2y, 0)~,
~(t2p1x, t2p1y, 0)~ to ~(t2p3x, t2p3y, dt)~, ~(t2p3x, t2p3y, dt)~ to
~(t2p4x, t2p4x, dt)~ and ~(t2p2x, t2p2y, 0)~ to ~(t2p4x, t2p4x, dt)~.
Allowed are either ~(t2p1x, t2p1y, 0)=(t2p2x, t2p2y, 0)~ or
~(t2p3x, t2p3y, dt)=(t2p4x, t2p4x, dt)~ but not both. Segments
~(t2p1x, t2p1y, 0)~ to ~(t2p2x, t2p2y, 0)~ and
~(t2p3x, t2p3y, dt)~ to ~(t2p4x, t2p4x, dt)~ are collinear.

$dt$ must be greater than $0$.

~detailedResult~ which will contain a numeric representation of the specific 
case
responsible for the return value of the function. (Full description of
~detailedResult~ should be added here.)

*/
static bool specialTrapeziumIntersects(
    double dt,
    double t1p1x,
    double t1p1y,
    double t1p2x,
    double t1p2y,
    double t1p3x,
    double t1p3y,
    double t1p4x,
    double t1p4y,
    double t2p1x,
    double t2p1y,
    double t2p2x,
    double t2p2y,
    double t2p3x,
    double t2p3y,
    double t2p4x,
    double t2p4y,
    unsigned int& detailedResult) {
    if (MRA_DEBUG)
        cerr << "specialTrapeziumIntersects() "
             << "(w/ detailedResult) called"
             << endl;

    detailedResult = 0;

    if (MRA_DEBUG) {
        cerr << "specialTrapeziumIntersects() trapezium 1: ("
             << t1p1x << ", " << t1p1y
             << "), (" << t1p2x << ", " << t1p2y
             << "), (" << t1p3x << ", " << t1p3y
             << "), (" << t1p4x << ", " << t1p4y
             << ")"
             << endl;
        cerr << "specialTrapeziumIntersects() trapezium 2: ("
             << t2p1x << ", " << t2p1y
             << "), (" << t2p2x << ", " << t2p2y
             << "), (" << t2p3x << ", " << t2p3y
             << "), (" << t2p4x << ", " << t2p4y
             << ")"
             << endl;
    }

/*
First, lets check the bounding boxes in the $(x, y)$-plane of the two
trapeziums.

*/
    double t1MinX;
    double t1MaxX;
    double t1MinY;
    double t1MaxY;

    minmax4(t1p1x, t1p2x, t1p3x, t1p4x, t1MinX, t1MaxX);
    minmax4(t1p1y, t1p2y, t1p3y, t1p4y, t1MinY, t1MaxY);

    double t2MinX;
    double t2MaxX;
    double t2MinY;
    double t2MaxY;

    minmax4(t2p1x, t2p2x, t2p3x, t2p4x, t2MinX, t2MaxX);
    minmax4(t2p1y, t2p2y, t2p3y, t2p4y, t2MinY, t2MaxY);

    if (MRA_DEBUG) {
        cerr << "specialTrapeziumIntersects() t1MinX=" << t1MinX
             << " t1MaxX=" << t1MaxX
             << " t1MinY=" << t1MinY
             << " t1MaxY=" << t1MaxY
             << endl;
        cerr << "specialTrapeziumIntersects() t2MinX=" << t2MinX
             << " t2MaxX=" << t2MaxX
             << " t2MinY=" << t2MinY
             << " t2MaxY=" << t2MaxY
             << endl;
    }

    if (lower(t1MaxX, t2MinX)
        || lower(t2MaxX, t1MinX)
        || lower(t1MaxY, t2MinY)
        || lower(t2MaxY, t1MinY)) {
        if (MRA_DEBUG)
            cerr << "specialTrapeziumIntersects() no bbox overlap"
                 << endl;

        detailedResult = 1;
        return false;
    }

/*
Now, lets see if the trapeziums touch in one segment.

*/
    if ((nearlyEqual(t1p1x, t2p1x)
         && nearlyEqual(t1p1y, t2p1y)
         && nearlyEqual(t1p3x, t2p3x)
         && nearlyEqual(t1p3y, t2p3y))
        || (nearlyEqual(t1p1x, t2p2x)
            && nearlyEqual(t1p1y, t2p2y)
            && nearlyEqual(t1p3x, t2p4x)
            && nearlyEqual(t1p3y, t2p4y))
        || (nearlyEqual(t1p2x, t2p1x)
            && nearlyEqual(t1p2y, t2p1y)
            && nearlyEqual(t1p4x, t2p3x)
            && nearlyEqual(t1p4y, t2p3y))
        || (nearlyEqual(t1p2x, t2p2x)
            && nearlyEqual(t1p2y, t2p2y)
            && nearlyEqual(t1p4x, t2p4x)
            && nearlyEqual(t1p4y, t2p4y))) {
        if (MRA_DEBUG)
            cerr << "specialTrapeziumIntersects() touching"
                 << endl;

        detailedResult = 2;
        return false;
    }

/*
The bounding boxes of the trapeziums overlap but they do not touch in one
segment. To determine if they actually intersect, we first calculate the
intersection of the two planes spanned by the trapezium.

Create equations for the two planes spanned by the trapeziums,
considering that one edge of
each trapezium may be a single point: Plane 1 is
$T1B+T1A\cdot (s, t)$ and Plane 2 is $T2B+T1A\cdot (s', t')$.

*/
    double T1A[3][2];
    double T1B[3];

    if (nearlyEqual(t1p1x, t1p2x) && nearlyEqual(t1p1y, t1p2y)) {
        T1A[0][0] = t1p4x-t1p3x;
        T1A[1][0] = t1p4y-t1p3y;
        T1A[2][0] = 0;

        T1A[0][1] = t1p1x-t1p3x;
        T1A[1][1] = t1p1y-t1p3y;
        T1A[2][1] = -dt;

        T1B[0] = t1p3x;
        T1B[1] = t1p3y;
        T1B[2] = dt;
    } else {
        T1A[0][0] = t1p2x-t1p1x;
        T1A[1][0] = t1p2y-t1p1y;
        T1A[2][0] = 0;

        T1A[0][1] = t1p3x-t1p1x;
        T1A[1][1] = t1p3y-t1p1y;
        T1A[2][1] = dt;

        T1B[0] = t1p1x;
        T1B[1] = t1p1y;
        T1B[2] = 0;
    }

    double T2A[3][2];
    double T2B[3];

    if (nearlyEqual(t2p1x, t2p2x) && nearlyEqual(t2p1y, t2p2y)) {
        T2A[0][0] = t2p4x-t2p3x;
        T2A[1][0] = t2p4y-t2p3y;
        T2A[2][0] = 0;

        T2A[0][1] = t2p1x-t2p3x;
        T2A[1][1] = t2p1y-t2p3y;
        T2A[2][1] = -dt;

        T2B[0] = t2p3x;
        T2B[1] = t2p3y;
        T2B[2] = dt;
    } else {
        T2A[0][0] = t2p2x-t2p1x;
        T2A[1][0] = t2p2y-t2p1y;

        T2A[2][0] = 0;

        T2A[0][1] = t2p3x-t2p1x;
        T2A[1][1] = t2p3y-t2p1y;
        T2A[2][1] = dt;

        T2B[0] = t2p1x;
        T2B[1] = t2p1y;
        T2B[2] = 0;
    }

/*
Create a linear system of equations $A$ and $B$, which represents
$T1B+T1A\cdot (s, t)=T2B+T2A\cdot (s', t')$. Apply Gaussian elimination to
$A$ and $B$.

*/
    double A[3][4];
    double B[3];
    double* Ap[3];

    for (unsigned int i = 0; i < 3; i++) {
        A[i][0] = T1A[i][0];
        A[i][1] = T1A[i][1];
        A[i][2] = -T2A[i][0];
        A[i][3] = -T2A[i][1];
        B[i] = T2B[i]-T1B[i];
        Ap[i] = A[i];
    }

    GaussTransform(3, 4, Ap, B);

/*
Now, the linear system of equation has the following format
\begin{eqnarray}
\begin{array}{ccc}
\begin{array}{cccc}
\ast & \ast & \ast & \ast \\
0    & \ast & \ast & \ast \\
0    & 0    & c1   & c2
\end{array} & \left| \begin{array}{c}
\ast \\
\ast \\
b
\end{array} \right.
\end{array}
\nonumber\end{eqnarray}
The asterisk denotes arbitrary values.

If $c1=0$, $c2=0$ and $b\ne0$, the two planes are parallel and not identical:
They do not intersect. If $c1=0$, $c2=0$ and $b=0$, the two planes are
identical and we have to check if the two trapeziums overlap.

*/
    if (nearlyEqual(Ap[2][2], 0.0) && nearlyEqual(Ap[2][3], 0.0)) {
        if (nearlyEqual(B[2], 0.0)) {
/*
They overlap if one of the segments of each trapezium, which are not
parallel to the $(x, y)$-plane, intersect with another of such segments
of the other trapezium.

*/
            if (MRA_DEBUG)
                cerr << "specialTrapeziumIntersects() "
                     << "identical plane"
                     << endl;

            if (specialSegmentIntersects1(
                    dt,
                    t1p1x, t1p1y, t1p3x, t1p3y,
                    t2p1x, t2p1y, t2p3x, t2p3y)
                || specialSegmentIntersects1(
                       dt,
                       t1p1x, t1p1y, t1p3x, t1p3y,
                       t2p2x, t2p2y, t2p4x, t2p4y)
                || specialSegmentIntersects1(
                       dt,
                       t1p2x, t1p2y, t1p4x, t1p4y,
                       t2p1x, t2p1y, t2p3x, t2p3y)
                || specialSegmentIntersects1(
                       dt,
                       t1p2x, t1p2y, t1p4x, t1p4y,
                       t2p2x, t2p2y, t2p4x, t2p4y)) {
                if (MRA_DEBUG)
                    cerr << "specialTrapeziumIntersects() intersects"
                         << endl;

                cerr << ::std::fixed
                     << ::std::setprecision(6);
                for (unsigned int i = 0; i < 3; i++) 
                    cerr <<Ap[i][0]<<" "
                         <<Ap[i][1]<<" "
                         <<Ap[i][2]<<" "
                         <<Ap[i][3] 
                         <<"|"
                         <<B[i]
                         <<endl;

                detailedResult = 3;
                return true;
            } else {
                if (MRA_DEBUG)
                    cerr << "specialTrapeziumIntersects() "
                         << "do not intersect"
                         << endl;

                detailedResult = 4;
                return false;
            }
        } else {
            if (MRA_DEBUG)
                cerr << "specialTrapeziumIntersects() parallel"
                     << endl;

            detailedResult = 5;
            return false;
        }
    }

/*
Otherwise, the two planes intersect in a line, which we need to
calculate now. We have to consider the following cases: For $c1=0$, it
follows that $t'=b/c2$ with $s'$ arbitrary; for $c2=0$, it follows that
$s'=b/c1$ with $t'$ arbitrary; otherwise, it follows that
$s'=(b-c2\cdot t')/c1$ with $t'$ arbitrary.

Inserting $s'$ and $t'$ into the equation of the plane spanned by
trapezium 2 provides the equation of the intersection line, which we
will use in notation $P+Q\cdot u$.

*/

    if (MRA_DEBUG)
        cerr << "specialTrapeziumIntersects() intersect"
             << endl;

    double P[3];
    double Q[3];

    if (nearlyEqual(Ap[2][2], 0.0)) {
/*
Case 1: $c1=0$.

*/

        double f = B[2]/Ap[2][3]; // = b/c2

        for (unsigned int i = 0; i < 3; i++) {
            P[i] = T2B[i]+T2A[i][1]*f;
            Q[i] = T2A[i][0];
        }
    } else if (nearlyEqual(Ap[2][3], 0.0)) {
/*
Case 2: $c2=0$.

*/

        double f = B[2]/Ap[2][2]; // = b/c1

        for (unsigned int i = 0; i < 3; i++) {
            P[i] = T2B[i]+T2A[i][0]*f;
            Q[i] = T2A[i][1];
        }
    } else {
/*
Case 3: $c1\ne 0$ and $c2\ne 0$.

Inserting $s'=(b-c2\cdot t')/c1$ into $T2B+T2A\cdot (s', t')$ yields
\begin{eqnarray}
\left(
\begin{array}{c}
T2B[0] \\
T2B[1] \\
T2B[2]
\end{array}
\right)+\left(
\begin{array}{c}
T2A[0][0] \\
T2A[1][0] \\
T2A[2][0]
\end{array}
\right)\cdot (b-c2\cdot t')/c1+\left(
\begin{array}{c}
T2A[0][1] \\
T2A[1][1] \\
T2A[2][1]
\end{array}
\right)\cdot t'
\nonumber\end{eqnarray}
and
\begin{eqnarray}
\left(
\begin{array}{c}
T2B[0]+T2A[0][0]\cdot b/c1 \\
T2B[1]+T2A[1][0]\cdot b/c1 \\
T2B[2]+T2A[2][0]\cdot b/c1
\end{array}
\right)+
\left(
\begin{array}{c}
 T2A[0][1]-T2A[0][0]\cdot c2/c1 \\
 T2A[1][1]-T2A[1][0]\cdot c2/c1 \\
 T2A[2][1]-T2A[2][0]\cdot c2/c1
\end{array}
\right)\cdot t'
\nonumber\end{eqnarray}

*/

        double f1 = B[2]/Ap[2][2];    // = b/c1
        double f2 = A[2][3]/Ap[2][2]; // = c2/c1

        if (MRA_DEBUG)
            cerr << "specialTrapeziumIntersects() f1="
                 << f1
                 << " f2="
                 << f2
                 << endl;

        for (unsigned int i = 0; i < 3; i++) {
            P[i] = T2B[i]+T2A[i][0]*f1;
            Q[i] = T2A[i][1]-T2A[i][0]*f2;
        }
    }

    if (MRA_DEBUG)
        cerr << "specialTrapeziumIntersects() ("
             << P[0]
             << ", "
             << P[1]
             << ", "
             << P[2]
             << ")+("
             << Q[0]
             << ", "
             << Q[1]
             << ", "
             << Q[2]
             << ")*u"
             << endl;

/*
Now, we have to check whether the intersection line intersects at least one
of the segments of each trapezium.

If $Q[2]=0$, the intersection line is parallel to the $(x, y)$-plane.

*/

    if (nearlyEqual(Q[2], 0.0)) {
/*
Check whether the intersection line is within the z-Range covered by the two
trapeziums.

*/
        if (MRA_DEBUG)
            cerr << "specialTrapeziumIntersects() intersection line "
                 << "parallel to (x, y)-plane"
                 << endl;

        if (lowerOrNearlyEqual(P[2], 0)
            || lowerOrNearlyEqual(dt, P[2])) {
            if (MRA_DEBUG)
                cerr << "specialTrapeziumIntersects() "
                     << "no intersection"
                     << endl;

            detailedResult = 6;
            return false;
        }

/*
Calculate the points where the trapeziums' segments intersect with the
intersection line.

*/

        double ip1x = t1p1x+(t1p3x-t1p1x)*P[2]/dt;
        double ip1y = t1p1y+(t1p3y-t1p1y)*P[2]/dt;

        double ip2x = t1p2x+(t1p4x-t1p2x)*P[2]/dt;
        double ip2y = t1p2y+(t1p4y-t1p2y)*P[2]/dt;

        double ip3x = t2p1x+(t2p3x-t2p1x)*P[2]/dt;
        double ip3y = t2p1y+(t2p3y-t2p1y)*P[2]/dt;

        double ip4x = t2p2x+(t2p4x-t2p2x)*P[2]/dt;
        double ip4y = t2p2y+(t2p4y-t2p2y)*P[2]/dt;


/*
Check for overlaps of the bounding boxes of the two intersection segments.
If they overlap, the sections intersect.

*/

        double ip1ip2MinX = ip1x < ip2x ? ip1x : ip2x;
        double ip1ip2MaxX = ip1x > ip2x ? ip1x : ip2x;

        double ip1ip2MinY = ip1y < ip2y ? ip1y : ip2y;
        double ip1ip2MaxY = ip1y > ip2y ? ip1y : ip2y;

        double ip3ip4MinX = ip3x < ip4x ? ip3x : ip4x;
        double ip3ip4MaxX = ip3x > ip4x ? ip3x : ip4x;

        double ip3ip4MinY = ip3y < ip4y ? ip3y : ip4y;
        double ip3ip4MaxY = ip3y > ip4y ? ip3y : ip4y;

        if (lower(ip1ip2MaxX, ip3ip4MinX)
            || lower(ip3ip4MaxX, ip1ip2MinX)
            || lower(ip1ip2MaxY, ip3ip4MinY)
            || lower(ip3ip4MaxY, ip1ip2MinY)) {
            if (MRA_DEBUG)
                cerr << "specialTrapeziumIntersects() "
                     << "no intersection"
                     << endl;

/*
This case should never happen.

*/

            assert(false);

            // return false;
        } else {
            if (MRA_DEBUG)
                cerr << "specialTrapeziumIntersects() intersection"
                     << endl;

            detailedResult = 7;
            return true;
        }
    } else {
/*
Intersection line hits $(x, y)$-plane in $(P[0], P[1], 0)$ and the
plane parallel to $(x, y)$-plane in distance $dt$ in
$(P[0]+Q[0], P[1]+Q[1], dt)$ as we have handled the case $Q[2]=0$
separately.

*/

        double z;

        double t1zMin = dt;
        double t1zMax = 0;
        bool t1Intersects = false;

        if (specialSegmentIntersects2(
                0, t1p1x, t1p1y, t1p2x, t1p2y, P, Q)) {
            t1zMin = t1zMin > 0 ? 0 : t1zMin;
            t1zMax = t1zMax < 0 ? 0 : t1zMax;
            t1Intersects = true;
        }
        if (specialSegmentIntersects2(
                dt, t1p3x, t1p3y, t1p4x, t1p4y, P, Q)) {
            t1zMin = t1zMin > dt ? dt : t1zMin;
            t1zMax = t1zMax < dt ? dt : t1zMax;
            t1Intersects = true;
        }
        if (specialSegmentIntersects1(
                dt,
                t1p1x, t1p1y, t1p3x, t1p3y,
                P[0], P[1], P[0]+Q[0], P[1]+Q[1])) {
            t1zMin = t1zMin > z ? z : t1zMin;
            t1zMax = t1zMax < z ? z : t1zMax;
            t1Intersects = true;
        }
        if (specialSegmentIntersects1(
                dt,
                t1p2x, t1p2y, t1p4x, t1p4y,
                P[0], P[1], P[0]+Q[0], P[1]+Q[1],
                z)) {
            t1zMin = t1zMin > z ? z : t1zMin;
            t1zMax = t1zMax < z ? z : t1zMax;
            t1Intersects = true;
        }

        double t2zMin = dt;
        double t2zMax = 0;

        bool t2Intersects = false;

        if (specialSegmentIntersects2(
                0, t2p1x, t2p1y, t2p2x, t2p2y, P, Q)) {
            t2zMin = t2zMin > 0 ? 0 : t2zMin;
            t2zMax = t2zMax < 0 ? 0 : t2zMax;
            t2Intersects = true;
        }
        if (specialSegmentIntersects2(
                dt, t2p3x, t2p3y, t2p4x, t2p4y, P, Q)) {
            t2zMin = t2zMin > dt ? dt : t2zMin;
            t2zMax = t2zMax < dt ? dt : t2zMax;
            t2Intersects = true;
        }
        if (specialSegmentIntersects1(
                dt,
                t2p1x, t2p1y, t2p3x, t2p3y,
                P[0], P[1], P[0]+Q[0], P[1]+Q[1],
                z)) {
            t2zMin = t2zMin > z ? z : t2zMin;
            t2zMax = t2zMax < z ? z : t2zMax;
            t2Intersects = true;
        }
        if (specialSegmentIntersects1(
                dt,
                t2p2x, t2p2y, t2p4x, t2p4y,
                P[0], P[1], P[0]+Q[0], P[1]+Q[1],
                z)) {
            t2zMin = t2zMin > z ? z : t2zMin;
            t2zMax = t2zMax < z ? z : t2zMax;
            t2Intersects = true;
        }

        if (MRA_DEBUG) {
            cerr << "specialTrapeziumIntersects() t1Intersects="
                 << t1Intersects
                 << endl;
            if (t1Intersects)
                cerr << "specialTrapeziumIntersects() t1zMin="
                     << t1zMin
                     << " t1zMax="
                     << t1zMax
                     << endl;
            cerr << "specialTrapeziumIntersects() t2Intersects="
                 << t2Intersects
                 << endl;
            if (t2Intersects)
                cerr << "specialTrapeziumIntersects() t2zMin="
                     << t2zMin
                     << " t2zMax="
                     << t2zMax
                     << endl;
        }

        if (t1Intersects
            && t2Intersects
            && !(lower(t1zMax, t2zMin) || lower(t2zMax, t1zMin))
            && !(nearlyEqual(t1zMax, 0.0)
                 && nearlyEqual(t1zMin, 0.0)
                 && nearlyEqual(t2zMax, 0.0)
                 && nearlyEqual(t2zMin, 0.0))
            && !(nearlyEqual(t1zMax, dt)
                 && nearlyEqual(t1zMin, dt)
                 && nearlyEqual(t2zMax, dt)
                 && nearlyEqual(t2zMin, dt))) {
            if (MRA_DEBUG)
                cerr << "specialTrapeziumIntersects() intersect"
                     << endl;

            detailedResult = 8;
            return true;
        } else {
            if (MRA_DEBUG)
                cerr << "specialTrapeziumIntersects() "
                     << "no intersection"
                     << endl;

            detailedResult = 9;
            return false;
        }
    }
}

/*
1.1 Other intersections

This section contain functions which examine the intersection between two
different object types (such as trapezium and segment).

1.1.1 Function ~specialInsidePointTrapezium()~

Return ~true~ if the point $(x, y, t)$ is within the trapezium spanned by the
segments $(l1p1x, l1p1y, t1)$ to $(l1p2x, l1p2y, t1)$ and
$(l2p1x, l2p1y, t2)$ to $(l2p2x, l2p2y, t2)$.

*/
bool specialInsidePointTrapezium(double t,
                                 double x,
                                 double y,
                                 double t1,
                                 double l1p1x,
                                 double l1p1y,
                                 double l1p2x,
                                 double l1p2y,
                                 double t2,
                                 double l2p1x,
                                 double l2p1y,
                                 double l2p2x,
                                 double l2p2y) {
    if (MRA_DEBUG) cerr << "sIPT() called" << endl;

/*
Check if $t1 <= t <= t2$ or $t2 <= t < t1$ holds. If not, no intersection
can occur.

*/
    if (!(lowerOrNearlyEqual(t1, t)
          && lowerOrNearlyEqual(t, t2))
        && !(lowerOrNearlyEqual(t2, t)
             && lowerOrNearlyEqual(t, t1))) {
        if (MRA_DEBUG) cerr << "sIPT() t check failed" << endl;

        return false;
    }

    if (MRA_DEBUG) cerr << "sIPT() t check succeeded" << endl;

/*
Calculate the points $(x1, y1, t)$ and $(x2, y2, t)$, which are the end
points of a segment parallel to $(l1p1x, l1p1y, t1)$ to $(l1p2x, l1p2y, t1)$
and ($l2p1x, l2p1y, t2$ to $(l2p2x, l2p2y, t2)$ and which is located between
these two segments.

*/
    double x1;
    double y1;
    double x2;
    double y2;

    if (nearlyEqual(t1, t2)) {
        x1 = l1p1x;
        y1 = l1p1y;
        x2 = l1p2x;
        y2 = l1p2y;
    } else {
        double f = (t-t1)/(t2-t1);

        if (MRA_DEBUG) cerr << "sIPT() f=" << f << endl;

        x1 = l1p1x+(l2p1x-l1p1x)*f;
        y1 = l1p1y+(l2p1y-l1p1y)*f;
        x2 = l1p2x+(l2p2x-l1p2x)*f;
        y2 = l1p2y+(l2p2y-l1p2y)*f;
    }

    if (MRA_DEBUG)
        cerr << "sIPT() x1=" << x1
             << " y1=" << y1
             << " x2=" << x2
             << " y2=" << y2
             << endl;

/*
If $(x, y, t)$ is located on segment $(x1, y1, t)$ and $(x2, y2, t)$, the
point is inside the trapezium, and outside otherwise.

*/
    if (((lowerOrNearlyEqual(x1, x)
          && lowerOrNearlyEqual(x, x2))
         || (lowerOrNearlyEqual(x2, x)
             && lowerOrNearlyEqual(x, x1)))
        && ((lowerOrNearlyEqual(y1, y)
             && lowerOrNearlyEqual(y, y2))
            || (lowerOrNearlyEqual(y2, y)
                && lowerOrNearlyEqual(y, y1)))) {
        if (MRA_DEBUG) cerr << "sIPT() inside" << endl;

        return true;
    } else {
        if (MRA_DEBUG) cerr << "sIPT() not inside" << endl;

        return false;
    }
}

/*
1.1.1 Function ~specialIntersectionTrapeziumSegment()~

Calculates the intersection of a trapezium and a segment
in three-dimensional space $(x, y, t)$.

The trapezium is spanned by the points ~(l1p1x, l1p1y, t1)~,
~(l1p2x, l1p2y, t1)~, ~(l2p1x, l2p1y, t2)~ and
~(l2p2x, l2p2y, t2)~. ~(l1p1x, l1p1y, t1)=(l1p2x, l1p2y, t1)~ or
~(l2p1x, l2p1y, t2)=(l2p2x, l2p2y, t2)~ is allowed but not both.
~l1t=l2t~ is allowed.

The segment is spanned by the points ~(p1x, p1y, t1)~ and ~(p2x, p2y, t2)~.
~(p1x, p1y, t1)=(p2x, p2y, t1)~ is allowed.

~t1=t2~ is allowed.

*/

void specialIntersectionTrapeziumSegment(double t1,
                                         double t2,
                                         double l1p1x,
                                         double l1p1y,
                                         double l1p2x,
                                         double l1p2y,
                                         double l2p1x,
                                         double l2p1y,
                                         double l2p2x,
                                         double l2p2y,
                                         double p1x,
                                         double p1y,
                                         double p2x,
                                         double p2y,
                                         bool& ip1present,
                                         double& ip1x,
                                         double& ip1y,
                                         double& ip1t,
                                         bool& ip2present,
                                         double& ip2x,
                                         double& ip2y,
                                         double& ip2t) {
    if (MRA_DEBUG) {
        cerr << "sITS() called" << endl;
        cerr << "sITS() t1=" << t1
             << " t2=" << t2
             << " l1=[" << l1p1x << " " << l1p1y
             << " " << l1p2x << " " << l1p2y
             << "] l2=[" << l2p1x << " " << l2p1y
             << " " << l2p2x << " " << l2p2y
             << "] p=(" << p1x << " " << p1y
             << " " << p2x << " " << p2y << ")"
             << endl;
    }


/*
Check bounding boxes in $(x, y)$-plane of trapzium and segment. If they
do not overlap, trapezium and segment do not intersect.

*/

    double tMinX;
    double tMaxX;
    double tMinY;
    double tMaxY;

    minmax4(l1p1x, l1p2x, l2p1x, l2p2x, tMinX, tMaxX);
    minmax4(l1p1y, l1p2y, l2p1y, l2p2y, tMinY, tMaxY);

    double lMinX = p1x < p2x ? p1x : p2x;
    double lMaxX = p1x > p2x ? p1x : p2x;
    double lMinY = p1y < p2y ? p1y : p2y;
    double lMaxY = p1y > p2y ? p1y : p2y;

    if (lower(tMaxX, lMinX)
        || lower(lMaxX, tMinX)
        || lower(tMaxY, lMinY)
        || lower(lMaxY, tMinY)) {
        if (MRA_DEBUG) cerr << "sITS() no bbox overlap" << endl;

        ip1present = false;
        ip2present = false;

        return;
    }

/*
Create equations for the plane spanned by the trapezium and the line
through the segment. Keep in mind that the trapezium might be a triangle
and that the line might be point only.

The plane is $P1+P2\cdot (s, t)$, where $T1$ is a vector with three elements
and $T2$ a matrix with three rows and two columns, and the line is
$L1+L2\cdot u$, where both $L1$ and $L2$ are vectors with three elements.

The interesection between plane and line is the solution of the equation
$P2\cdot (s, t)-L2\cdot u=L1-T1$. Present the left handed side by the
matrix $A$ with three rows and three colums and the right handed side by
the verctor $B$ with three elements.

*/

    double A[3][3];
    double B[3];

    if (nearlyEqual(l1p1x, l1p2x) && nearlyEqual(l1p1y, l1p2y)) {
        B[0] = -l2p1x;
        B[1] = -l2p1y;
        B[2] = -t2;

        A[0][0] = l2p2x-l2p1x;
        A[1][0] = l2p2y-l2p1y;
        A[2][0] = 0;
        A[0][1] = l1p1x-l2p1x;
        A[1][1] = l1p1y-l2p1y;
        A[2][1] = t1-t2;
    } else {
        B[0] = -l1p1x;
        B[1] = -l1p1y;
        B[2] = -t1;

        A[0][0] = l1p2x-l1p1x;
        A[1][0] = l1p2y-l1p1y;
        A[2][0] = 0;
        A[0][1] = l2p1x-l1p1x;
        A[1][1] = l2p1y-l1p1y;
        A[2][1] = t2-t1;
    }

    B[0] += p1x;
    B[1] += p1y;
    B[2] += t1;

    A[0][2] = p1x-p2x;
    A[1][2] = p1y-p2y;
    A[2][2] = t1-t2;

/*
Apply Gauss elimination.

*/

    double* Ap[3];
    for (unsigned int i = 0; i < 3; i++) Ap[i] = A[i];

    GaussTransform(3, 3, Ap, B);

/*
Analyse solutions.

*/

    if (nearlyEqual(A[2][2], 0.0)) {
        if (nearlyEqual(B[2], 0.0)) {
/*
Segment in same plane as trapezium.

*/

            if (MRA_DEBUG) cerr << "sITS() in same plane" << endl;
/*
The following cases have to be considered: Segment entirely outside
trapezium, segment entirely inside trapezium, $(p1x, p1y, t1)$ inside
trapezium, $(p2x, p2y, t2)$ inside trapezium and $(p1x, p1y, t1)$ and
$(p2x, p2y, t2)$ outside trapezium but still intersecting with trapezium.

*/

            bool p1in =
                between(l1p1x, p1x, l1p2x)
                && between(l1p1y, p1y, l1p2y);
            bool p2in =
                between(l2p1x, p2x, l2p2x)
                && between(l2p1y, p2y, l2p2y);

            if (p1in && p2in) {
                if (MRA_DEBUG)
                    cerr << "sITS() entirely in trapezium"
                         << endl;

                ip1present = true;
                ip2present = true;

                ip1x = p1x;
                ip1y = p1y;
                ip1t = t1;

                ip2x = p2x;
                ip2y = p2y;
                ip2t = t2;

                return;
            } else {
                if (MRA_DEBUG)
                    cerr << "sITS() not entirely in trapezium"
                         << endl;

                double l1p1t;
                bool l1p1is =
                    specialSegmentIntersects1(
                        t2-t1,
                        l1p1x, l1p1y, l2p1x, l2p1y,
                        p1x, p1y, p2x, p2y,
                        l1p1t);

                double l1p2t;
                bool l1p2is =
                    specialSegmentIntersects1(
                        t2-t1,
                        l1p2x, l1p2y, l2p2x, l2p2y,
                        p1x, p1y, p2x, p2y,
                        l1p2t);

                if (p1in) {
                    if (MRA_DEBUG)
                        cerr << "sITS() p1 in trapezium"
                             << endl;

                    assert(l1p1is || l1p2is);

                    ip1present = true;
                    ip2present = true;

                    ip1x = p1x;
                    ip1y = p1y;
                    ip1t = t1;

                    if (l1p1is) {
                        ip2x = p1x+(p2x-p1x)*l1p1t/(t2-t1);
                        ip2y = p1y+(p2y-p1y)*l1p1t/(t2-t1);
                        ip2t = l1p1t;
                    } else {
                        ip2x = p1x+(p2x-p1x)*l1p2t/(t2-t1);
                        ip2y = p1y+(p2y-p1y)*l1p2t/(t2-t1);
                        ip2t = l1p2t;
                    }

                    return;
                } else if (p2in) {
                    if (MRA_DEBUG)
                        cerr << "sITS() p2 in trapezium"
                             << endl;

                    assert(l1p1is || l1p2is);

                    ip1present = true;
                    ip2present = true;

                    ip2x = p2x;
                    ip2y = p2y;
                    ip2t = t2;

                    if (l1p1is) {
                        ip1x = p1x+(p2x-p1x)*l1p1t/(t2-t1);
                        ip1y = p1y+(p2y-p1y)*l1p1t/(t2-t1);
                        ip1t = l1p1t;
                    } else {
                        ip1x = p1x+(p2x-p1x)*l1p2t/(t2-t1);
                        ip1y = p1y+(p2y-p1y)*l1p2t/(t2-t1);
                        ip1t = l1p2t;
                    }

                    return;
                } else {
                    if (MRA_DEBUG)
                        cerr << "sITS() neither p1 nor p2 in "
                             << "trapezium"
                             << endl;

                    if (l1p1is && l1p2is) {
                        if (MRA_DEBUG)
                            cerr << "sITS() intersects trapezium"
                                 << endl;

                        ip1present = true;
                        ip2present = true;

                        if (l1p1t < l1p2t) {
                            ip1x = p1x+(p2x-p1x)*l1p1t/(t2-t1);
                            ip1y = p1y+(p2y-p1y)*l1p1t/(t2-t1);
                            ip1t = l1p1t;

                            ip2x = p1x+(p2x-p1x)*l1p2t/(t2-t1);
                            ip2y = p1y+(p2y-p1y)*l1p2t/(t2-t1);
                            ip2t = l1p2t;
                        } else {
                            ip1x = p1x+(p2x-p1x)*l1p2t/(t2-t1);
                            ip1y = p1y+(p2y-p1y)*l1p2t/(t2-t1);
                            ip1t = l1p2t;

                            ip2x = p1x+(p2x-p1x)*l1p1t/(t2-t1);
                            ip2y = p1y+(p2y-p1y)*l1p1t/(t2-t1);
                            ip2t = l1p1t;
                        }

                        return;
                    } else {
                        if (MRA_DEBUG)
                            cerr << "sITS() entirely outside "
                                 << "trapezium"
                                 << endl;

                        assert(!l1p1is && !l1p2is);

                        ip1present = false;
                        ip2present = false;

                        return;
                    }
                }
            }

        } else {
/*
Segment in different plane as trapezium but parallel to trapezium.

*/
            if (MRA_DEBUG) cerr << "sITS() no solution" << endl;

            ip1present = false;
            ip2present = false;

            return;
        }
    } else {
/*
Segment not parallel to trapezium.

*/
        double u = B[2]/A[2][2];
        double x = p1x+(p2x-p1x)*u;
        double y = p1y+(p2y-p1y)*u;
        double t = t1+(t2-t1)*u;

        if (MRA_DEBUG)
            cerr << "sITS() intersection with u="
                 << u
                 << " (" << x << " " << y << " " << t << ")"
                 << endl;

        ip2present = false;

        if (specialInsidePointTrapezium(
                t, x, y,
                t1, l1p1x, l1p1y, l1p2x, l1p2y,
                t2, l2p1x, l2p1y, l2p2x, l2p2y)) {

            if (MRA_DEBUG)
                cerr << "sITS() intersection is inside" << endl;

            ip1present = true;

            ip1x = x;
            ip1y = y;
            ip1t = t;
        } else {
            if (MRA_DEBUG)
                cerr << "sITS() intersection is not inside" << endl;

            ip1present = false;
        }

        return;
    }
}

/*
1.1 Relative positions of points and segments

1.1.1 Function ~pointAboveSegment()~

Returns ~true~ if point $(x, y)$ is located above or on segment spanned by
the points $(p1x, p1y)$ and $(p2x, p2y)$.

*/

static bool pointAboveSegment(double x,
                              double y,
                              double p1x,
                              double p1y,
                              double p2x,
                              double p2y) {
    if (MRA_DEBUG)
        cerr << "pointAboveSegment() called p=(" << x << " " << y
             << ") p1=(" << p1x << " " << p1y
             << ") p2=(" << p2x << " " << p2y << ")"
             << endl;

    if (nearlyEqual(p1x, p2x)) {
        if (MRA_DEBUG)
            cerr << "pointAboveSegment() p1x=p2x" << endl;

        return x <= p1x;
    } else if (nearlyEqual(p1y, p2y)) {
        if (MRA_DEBUG)
            cerr << "pointAboveSegment() p1y=p2y" << endl;

        return y >= p1y;
    } else {
        if (MRA_DEBUG)
            cerr << "pointAboveSegment() other" << endl;

        double t = (x-p1x)/(p2x-p1x);
        double py = p1y+(p2y-p1y)*t;

        if (MRA_DEBUG)
            cerr << "pointAboveSegment() py=" << py << endl;

        return y >= py;
    }
}

/*
1.1.1 Function ~pointBelowSegment()~

Returns ~true~ if point $(x, y)$ is located below or on segment spanned by
the points $(p1x, p1y)$ and $(p2x, p2y)$.

*/

static bool pointBelowSegment(double x,
                              double y,
                              double p1x,
                              double p1y,
                              double p2x,
                              double p2y) {
    if (MRA_DEBUG)
        cerr << "pointBelowSegment() called p=(" << x << " " << y
             << ") p1=(" << p1x << " " << p1y
             << ") p2=(" << p2x << " " << p2y << ")"
             << endl;

    if (nearlyEqual(p1x, p2x)) {
        if (MRA_DEBUG)
            cerr << "pointBelowSegment() p1x=p2x" << endl;

        return x >= p1x;
    } else if (nearlyEqual(p1y, p2y)) {
        if (MRA_DEBUG)
            cerr << "pointBelowSegment() p1y=p2y" << endl;

        return y <= p1y;
    } else {
        if (MRA_DEBUG)
            cerr << "pointBelowSegment() other" << endl;

        double t = (x-p1x)/(p2x-p1x);
        double py = p1y+(p2y-p1y)*t;

        return y <= py;
    }
}

/*
1.1 Function ~restrictUPointToInterval()~

Create new ~UPoint~ instance in ~rUp~ from ~up~, which is restricted to
interval ~iv~.

*/

static void restrictUPointToInterval(const UPoint& up,
                                     const Interval<Instant> iv,
                                     UPoint& rUp) {
    if (MRA_DEBUG)
        cerr << "restrictUPointToInterval() called" << endl;

    if (nearlyEqual(iv.start.ToDouble(), iv.end.ToDouble()))
        rUp = up;
    else {
        double ti =
            (iv.start.ToDouble()
             -up.timeInterval.start.ToDouble())
            /(up.timeInterval.end.ToDouble()
              -up.timeInterval.start.ToDouble());
        double tf =
            (iv.end.ToDouble()
             -up.timeInterval.start.ToDouble())
            /(up.timeInterval.end.ToDouble()
              -up.timeInterval.start.ToDouble());

        UPoint dummy(
            iv,
            up.p0.GetX()+(up.p1.GetX()-up.p0.GetX())*ti,
            up.p0.GetY()+(up.p1.GetY()-up.p0.GetY())*ti,
            up.p0.GetX()+(up.p1.GetX()-up.p0.GetX())*tf,
            up.p0.GetY()+(up.p1.GetY()-up.p0.GetY())*tf);

        rUp = dummy;
    }
}

/*
1 Supporting classes

Supporting classes are those, which are not registered as SECONDO datatypes
but are used to implement the SECONDO datatypes ~intimeregion~, ~uregion~ and
~movingregion~.

1.1 Class ~MSegmentData~

This class is used to represent the segments, which are used to represent
region units in section \ref{uregion}.

1.1.1 Class definition

The enumeratation ~DegenMode~ specifies how to handle these segments of a
degenerated region unit, which are causing the degeneration (in this case,
we say that the segment is degenerated).

  * ~DGM\_UNKNOWN~: If is not known yet whether the segment is degenerated.
  * ~DGM\_NONE~: Segment is not degenerated.
  * ~DGM\_IGNORE~: Segment is degenerated and can be ignored because it is not
    a border of region area due to its degeneration.
  * ~DGM\_INSIDEABOVE~: Segment is degenerated but is still border of region
    area, which is left or above the segment.
  * ~DGM\_NOTINSIDEABOVE~: Segment is degenerated but is still border of
    region area, which is right or below the segment.

*/
enum DegenMode { DGM_UNKNOWN,
                 DGM_NONE,
                 DGM_IGNORE,
                 DGM_INSIDEABOVE,
                 DGM_NOTINSIDEABOVE };

class MSegmentData {
/*
The private attributes are used as follows:

  * ~faceno~: The number of the region's face to which the segment belongs.
  * ~cycleno~: The number of the face's cycle, to which the segment belongs.
  * ~segmentno~: The number of the segment in its cycle.
  * ~insideAbove~: ~true~, if the region's interior is left or above of the
    segment. Please note that this is independent of the degeneration status
    of the segment.
  * ~degeneratedInitialInitial~, ~degeneratedFinalFinal~: Specifies whether
    the segment degenerated in the initial or final instant. If so, these
    attributes indicate too whether it is still border of any region area
    even though it is degenerated.
  * ~degeneratedInitialNext~, ~degeneratedFinalNext~: If segments merge into
    a single segments due to degeneration, these attributes are the next
    merged degenerated segments in the region unit's list of segments. The
    last segment will have the value $-1$ in these attributes.
  * ~initialStartX~, ~initialStartY~, ~initialEndX~, ~initialEndY~: The
    coordinates of the segment's start and end points in the initial instant.
  * ~finalStartX~, ~finalStartY~, ~finalEndX~, ~finalEndY~: The
    coordinates of the segment's start and end points in the initial instant.
  * ~pointInitial~, ~pointFinal~: A segment might be reduced to a point in the
    initial or final instant (but not both). This is indicated by these
    values of these two attributes. Please note that reduction of a segment
    to a point and degeneration of a segment are different concepts.

The constructor assures that the segment in initial and final instant is
collinear.

*/

// *hm* Debug only, set attributes to private later again
// private:
public:
    unsigned int faceno;
    unsigned int cycleno;
    unsigned int segmentno;

    bool insideAbove;

    int degeneratedInitialNext;
    int degeneratedFinalNext;
    DegenMode degeneratedInitial;
    DegenMode degeneratedFinal;

    double initialStartX;
    double initialStartY;
    double initialEndX;
    double initialEndY;

    double finalStartX;
    double finalStartY;
    double finalEndX;
    double finalEndY;

    bool pointInitial;
    bool pointFinal;

    MSegmentData() {
        if (MRA_DEBUG)
            cerr << "MSegmentData::MSegmentData() #1 called" << endl;
    }

/*
This constructor sets the ~faceno~, ~cycleno~, ~segmentno~, ~insideAbove~,
~initialStartX~, ~initialStartY~, ~initialEndX~, ~initialEndY~,
~finalStartX~, ~finalStartY~, ~finalEndX~ and ~finalEndY~ attributes
according to the parameters.

~degeneratedInitialNext~ and ~degeneratedFinalNext~ are initialised with
$-1$, ~degeneratedInitial~ and ~degeneratedFinal~ with $DGM_UNKNOWN$.

~pointInitial~ and ~pointFinal~ are calculated from the parameters and
an exception is thrown if segment is reduced to a point in initial and
final instant.

It is assured that the segment is collinear in initial and final instant
and an exception is thrown otherwise.

*/
    MSegmentData(unsigned int fno,
                 unsigned int cno,
                 unsigned int sno,
                 bool ia,
                 double isx,
                 double isy,
                 double iex,
                 double iey,
                 double fsx,
                 double fsy,
                 double fex,
                 double fey) :
        faceno(fno),
        cycleno(cno),
        segmentno(sno),
        insideAbove(ia),
        degeneratedInitialNext(-1),
        degeneratedFinalNext(-1),
        degeneratedInitial(DGM_UNKNOWN),
        degeneratedFinal(DGM_UNKNOWN),
        initialStartX(isx),
        initialStartY(isy),
        initialEndX(iex),
        initialEndY(iey),
        finalStartX(fsx),
        finalStartY(fsy),
        finalEndX(fex),
        finalEndY(fey)  {
        if (MRA_DEBUG)
            cerr << "MSegmentData::MSegmentData() #2 "
                 << "called counter=["
                 << faceno << " " << cycleno << segmentno
                 << "] flags=["
                 << insideAbove
                 << " " << degeneratedInitialNext
                 << " " << degeneratedFinalNext
                 << "] initial=["
                 << initialStartX << " " << initialStartY
                 << " "
                 << initialEndX << " " << initialEndY
                 << "] final=["
                 << finalStartX << " " << finalStartY
                 << " "
                 << finalEndX << " " << finalEndY
                 << "]"
                 << endl;

/*
Calculate whether segment is point in initial or final instant.

*/
        pointInitial =
            nearlyEqual(isx, iex) && nearlyEqual(isy, iey);
        pointFinal =
            nearlyEqual(fsx, fex) && nearlyEqual(fsy, fey);

/*
Check whether initial and final segment are collinear,

*/
        bool collinear;

        if (pointInitial && pointFinal) {
/*
Error: A segment may not be reduced to a point both in initial and final
instant.

*/
            if (MRA_DEBUG)
                cerr << "MSegmentData::MSegmentData() both reduced"
                     << endl;

            throw invalid_argument("both initial and final segment "
                                   "reduced to point, which is not "
                                   "allowed");
        } else if (pointInitial) {
/*
Only initial segment reduced to point. Initial and final segment are trivially
collinear.

*/
            if (MRA_DEBUG)
                cerr << "MSegmentData::MSegmentData() "
                     << "initial reduced"
                     << endl;

            collinear = true;
        } else if (pointFinal) {
/*
Only final segment reduced to point. Initial and final segment are trivially
collinear.

*/
            if (MRA_DEBUG)
                cerr << "MSegmentData::MSegmentData() "
                     << "final reduced"
                     << endl;

            collinear = true;
        } else if (nearlyEqual(isx, iex) && nearlyEqual(fsx, fex)) {
/*
Both segments are vertical. Check if both segments have the same
orientation.

*/
            if (MRA_DEBUG)
                cerr << "MSegmentData::MSegmentData() "
                     << "both vertical"
                     << endl;

            collinear =
                (lowerOrNearlyEqual(isy, iey)
                 && lowerOrNearlyEqual(fsy, fey))
                || (greaterOrNearlyEqual(isy, iey)
                    && greaterOrNearlyEqual(fsy, fey));
        } else if (nearlyEqual(isx, iex) || nearlyEqual(fsx, fex)) {
/*
Only initial or final segment is vertical but not both.

*/
            if (MRA_DEBUG)
                cerr << "MSegmentData::MSegmentData() "
                     << "one vertical" << endl;

            collinear = false;
        } else {
/*
Both segments are not vertical.

*/
            if (MRA_DEBUG)
                cerr << "MSegmentData::MSegmentData() "
                     << "none vertical"
                     << endl;

#ifdef SCHMUH
            double id = (iey-isy)/(iex-isx);
            double fd = (fey-fsy)/(fex-fsx);

            if (MRA_DEBUG)
                cerr << setprecision(10)
                     << "MSegmentData::MSegmentData() id="
                     << id << " fd=" << fd
                     << endl;

            collinear = nearlyEqual(id, fd);
#endif

            collinear = nearlyEqual((iey-isy)*(fex-fsx), (fey-fsy)*(iex-isx));

            if (!collinear) {
                cerr << ::std::fixed << ::std::setprecision(6);
                cerr << "isx=" << isx
                     << " isy=" << isy
                     << " iex=" << iex
                     << " iey=" << iey
                     << " fsx=" << fsx
                     << " fsy=" << fsy
                     << " fex=" << fex
                     << " fey=" << fey
                     << endl;
                cerr << "(iey-isy)/(iex-isx)=" << (iey-isy)/(iex-isx) << endl;
                cerr << "(fey-fsy)/(fex-fsx)=" << (fey-fsy)/(fex-fsx) << endl;
                cerr << "(iey-isy)*(fex-fsx)=" << (iey-isy)*(fex-fsx) << endl;
                cerr << "(fey-fsy)*(iex-isx)=" << (fey-fsy)*(iex-isx) << endl;
            }
        }

        if (!collinear)
            throw
                invalid_argument(
                    "initial and final segment not collinear");
    }

/*
Attribute access methods.

*/
    unsigned int GetFaceNo(void) const { return faceno; }
    unsigned int GetCycleNo(void) const { return cycleno; }
    unsigned int GetSegmentNo(void) const { return segmentno; }
    double GetInitialStartX(void) const { return initialStartX; }
    double GetInitialStartY(void) const { return initialStartY; }
    double GetInitialEndX(void) const { return initialEndX; }
    double GetInitialEndY(void) const { return initialEndY; }
    double GetFinalStartX(void) const { return finalStartX; }
    double GetFinalStartY(void) const { return finalStartY; }
    double GetFinalEndX(void) const { return finalEndX; }
    double GetFinalEndY(void) const { return finalEndY; }
    bool GetInsideAbove(void) const { return insideAbove; }

/*
Generate new ~MSegmentData~ instant in ~rDms~ from current instant, where the
original interval ~origIv~ has been restricted to ~restrIv~.

*/
    void restrictToInterval(Interval<Instant> origIv,
                            Interval<Instant> restrIv,
                            MSegmentData& rDms) const;

/*
Since no other classes are accessing the private attributes of
~MSegmentData~, we declare ~URegion~ as friend. This is shorter,
yet somewhat clumsy than creating attribute access functions.

*/
    // *hm* is there a better solution?
    friend class URegion;

};

/*
1.1.1 Method ~MSegmentData::restrictToInterval()~

*/
void MSegmentData::restrictToInterval(Interval<Instant> origIv,
                                      Interval<Instant> restrIv,
                                      MSegmentData& rDms) const {
    if (MRA_DEBUG)
        cerr << "MSegmentData::restrictToInterval() called" << endl;

/*
Implementation is straightforward: Copy the attributes, which do not need to
be adjusted, and calculate the new initial and final end points.

*/
    rDms.faceno = faceno;
    rDms.cycleno = cycleno;
    rDms.segmentno = segmentno;

    rDms.insideAbove = insideAbove;

    rDms.degeneratedInitialNext = degeneratedInitialNext;
    rDms.degeneratedFinalNext = degeneratedFinalNext;
    rDms.degeneratedInitial = degeneratedInitial;
    rDms.degeneratedFinal = degeneratedFinal;

    rDms.pointInitial = pointInitial;
    rDms.pointFinal = pointFinal;

    if (nearlyEqual(origIv.end.ToDouble(),
                    origIv.start.ToDouble())) {
        rDms.initialStartX = initialStartX;
        rDms.initialStartY = initialStartY;
        rDms.initialEndX = initialEndX;
        rDms.initialEndY = initialEndY;
        rDms.finalStartX = finalStartX;
        rDms.finalStartY = finalStartY;
        rDms.finalEndX = finalEndX;
        rDms.finalEndY = finalEndY;
    } else {
        double ti =
            (restrIv.start.ToDouble()-origIv.start.ToDouble())
            /(origIv.end.ToDouble()-origIv.start.ToDouble());
        double tf =
            (restrIv.end.ToDouble()-origIv.start.ToDouble())
            /(origIv.end.ToDouble()-origIv.start.ToDouble());

        if (MRA_DEBUG)
            cerr << "MSegmentData::restrictToInterval() ti="
                 << ti << " tf=" << tf
                 << endl;

        rDms.initialStartX =
            initialStartX+(finalStartX-initialStartX)*ti;
        rDms.initialStartY =
            initialStartY+(finalStartY-initialStartY)*ti;
        rDms.initialEndX =
            initialEndX+(finalEndX-initialEndX)*ti;
        rDms.initialEndY =
            initialEndY+(finalEndY-initialEndY)*ti;
        rDms.finalStartX =
            initialStartX+(finalStartX-initialStartX)*tf;
        rDms.finalStartY =
            initialStartY+(finalStartY-initialStartY)*tf;
        rDms.finalEndX =
            initialEndX+(finalEndX-initialEndX)*tf;
        rDms.finalEndY =
            initialEndY+(finalEndY-initialEndY)*tf;
    }
}

/*
1.1 Class ~TrapeziumSegmentIntersection~

Represents an intersection point as used by the ~intersection~ operator.

The enumeration ~TsiType~ denotes whether an intersection will be used
($TSI\_ENTER$ when a point enters a moving region at the intersection
point and $TSI\_LEAVE$ when the point leaves the moving region at the
intersection point) or will be ignored due to degeneration
related special cases ($TSI\_IGNORE$).

*/
enum TsiType { TSI_ENTER, TSI_LEAVE, TSI_IGNORE };

class TrapeziumSegmentIntersection {
public:
/*
~type~ is the type of the intersection as described by above shown
enumeration. ~x~ and ~y~ are the coordinates of the intersection
and ~t~ the time of the intersection.

*/
    TsiType type;

    double x;
    double y;
    double t;

/*
Compares intersections by their time.

*/
    bool operator<(const TrapeziumSegmentIntersection& tsi) const {
        if (nearlyEqual(t, tsi.t)) {
            return type < tsi.type;
        } else
            return t < tsi.t;
    }
};

/*
1.1 Class template ~RefinementPartition~

This class calculates a list of time intervals from two ~Mapping~ instances
~m1~ and ~m2~. For each of these intervals, one of the following three
conditions holds:

  1 The interval is a sub-interval of a unit's interval for both ~m1~
    and ~m2~.

  2 The interval is a sub-interval of a unit's interval for ~m1~ but
    the intersection with each interval of a units in ~m2~ is empty.

  3 The interval is a sub-interval of a unit's interval for ~m2~ but
    the intersection with each interval of a units in ~m1~ is empty.

(An interval $i_1$ is a sub-interval of interval $i_2$ if and only if
$i_1 \subseteq i_2$.)

This list is called \underline{refinement partition}.

The classes used for ~Mapping1~ and ~Mapping2~ must inherit from class
~Mapping~ from the ~TemporalAlgebra~. The classes user for ~Unit1~ and
~Unit2~ must inherit from class ~Unit~ in ~TemporalAlgebra~.

1.1.1 Class template definition

*/

template<class Mapping1, class Mapping2, class Unit1, class Unit2>
class RefinementPartition {
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
    vector< Interval<Instant>* > iv;
    vector<int> vur;
    vector<int> vup;

/*
~AddUnit()~ is a small helper method to create a new interval from
~start~ and ~end~ instant and ~lc~ and ~rc~ flags and to add these to the
~iv~, ~vur~ and ~vup~ vectors.

*/
    void AddUnits(const int urPos,
                  const int upPos,
                  const Instant& start,
                  const Instant& end,
                  const bool lc,
                  const bool rc);

public:
/*
The constructor creates the refinement partition from the two ~Mapping~
instances ~mr~ and ~mp~.

Runtime is $O(\max(n, m))$ with $n$ and $m$ the numbers of units in
~mr~ and ~mp~.

*/
    RefinementPartition(Mapping1& mr, Mapping2& mp);

/*
Since the elements of ~iv~ point to dynamically allocated objects, we need
a destructor.

*/
    ~RefinementPartition();

/*
Return the number of intervals in the refinement partition.

*/
    unsigned int Size(void ) {
        if (MRA_DEBUG)
            cerr << "RP::Size() called" << endl;

        return iv.size();
    }

/*
Return the interval and indices in original units of position $pos$ in
the refinement partition in the referenced variables ~civ~, ~ur~ and
~up~. Remember that ~ur~ or ~up~ may be $-1$ if interval is no sub-interval
of unit intervals in the respective ~Mapping~ instance.

Runtime is $O(1)$.

*/
    void Get(unsigned int pos,
             Interval<Instant>*& civ,
             int& ur,
             int& up) {
        if (MRA_DEBUG)
            cerr << "RP::Get() called" << endl;

        assert(pos < iv.size());

        civ = iv[pos];
        ur = vur[pos];
        up = vup[pos];
    }
};

/*
1.1.1 Constructor template

*/

template<class Mapping1, class Mapping2, class Unit1, class Unit2>
RefinementPartition<Mapping1, Mapping2, Unit1, Unit2>
::RefinementPartition(
    Mapping1& mr,
    Mapping2& mp) {
    if (MRA_DEBUG)
        cerr << "RP::RP() called" << endl;

/*
To construct the refinement partition, we will move ~mrUnit~ and
~mpUnit~ through the units of ~mr~ and ~mp~. ~mrUnit~ and ~mpUnit~ will
be increased when the unit's interval has been added to the refinement
partition.

*/
    int mrUnit = 0;
    int mpUnit = 0;

/*
~ur~ and ~up~ will hold the units with the indeces ~mrUnit~ and ~mpUnit~.

*/
    Unit1 ur;
    const Unit2 *up;

    mr.Get(0, ur);
    mp.Get(0, up);

/*
~t~ will hold the right border of the last interval added to the refinement
position and ~c~ will indicate if this interval was open or closed. To avoid
special treatment of the first interval, ~t~ is initialised with the values
below.

*/
    Instant t;
    bool c;

    if (ur.timeInterval.start < up->timeInterval.start) {
        t = ur.timeInterval.start;
        c = !ur.timeInterval.lc;
    } else {
        t = up->timeInterval.start;
        c = !up->timeInterval.lc;
    }

/*
As long as we did not reach the end of units in ~mr~ or ~mp~...

*/
    while (mrUnit < mr.GetNoComponents()
           && mpUnit < mp.GetNoComponents()) {
        if (MRA_DEBUG) {
            cerr << "RP::RP() mrUnit="
                 << mrUnit
                 << " mpUnit="
                 << mpUnit
                 << " t="
                 << t.ToDouble()
                 << endl;
            cerr << "RP::RP() mrUnit interval=["
                 << ur.timeInterval.start.ToDouble()
                 << " "
                 << ur.timeInterval.end.ToDouble()
                 << " "
                 << ur.timeInterval.lc
                 << " "
                 << ur.timeInterval.rc
                 << "]"
                 << endl;
            cerr << "RP::RP() mpUnit interval=["
                 << up->timeInterval.start.ToDouble()
                 << " "
                 << up->timeInterval.end.ToDouble()
                 << " "
                 << up->timeInterval.lc
                 << " "
                 << up->timeInterval.rc
                 << "]"
                 << endl;
        }

/*
We now have to analyse the relative positions of the two intervals in the
current units of ~mr~ and ~mp~. This is done by considering a number of
different cases.

*/

        // graphical representation of intervals in debug output
        // [---[  interval closed on left side, open on right side
        // ]---|  interval open on left side, open or closed status
        //        of right side not relevant

        if (ur.timeInterval.start.Compare(
                &up->timeInterval.start) == 0
            && ur.timeInterval.end.Compare(
                   &up->timeInterval.end) == 0) {
/*
\textbf{Case 1:} Both intervals have the same start and end time.

*/
            if (MRA_DEBUG) {
                cerr << "RP::RP()   ur: |-----|" << endl;
                cerr << "RP::RP()   up: |-----|" << endl;
            }

/*
If one of the intervals is open and one is closed on the left side,
we have to add a point interval to the refinement partition.

*/
            if (!(ur.timeInterval.lc && up->timeInterval.lc)) {
                if (ur.timeInterval.lc)
                    AddUnits(
                        mrUnit,
                        -1,
                        ur.timeInterval.start,
                        ur.timeInterval.start,
                        true,
                        true);
                else if (up->timeInterval.lc)
                    AddUnits(
                        -1,
                        mpUnit,
                        ur.timeInterval.start,
                        ur.timeInterval.start,
                        true,
                        true);
            }

/*
Add interval. It is closed on the left (right) side if both unit
intervals are closed on the left (right) side.

*/
            AddUnits(
                mrUnit,
                mpUnit,
                ur.timeInterval.start,
                ur.timeInterval.end,
                ur.timeInterval.lc && up->timeInterval.lc,
                ur.timeInterval.rc && up->timeInterval.rc);

/*
If one of the intervals is open and one is closed on the right side,
we have to add a point interval to the refinement partition. This determines
the value of ~c~ too, which we use to remember the open/closed flag of the
last interval added to the refinement partition.

*/
            if (!(ur.timeInterval.rc && up->timeInterval.rc)) {
                if (ur.timeInterval.rc) {
                    AddUnits(
                        mrUnit,
                        -1,
                        ur.timeInterval.end,
                        ur.timeInterval.end,
                        true,
                        true);
                    c = true;
                } else if (up->timeInterval.rc) {
                    AddUnits(
                        -1,
                        mpUnit,
                        ur.timeInterval.end,
                        ur.timeInterval.end,
                        true,
                        true);
                    c = true;
                } else
                    c = false;
            } else
                c = false;

/*
Remember right border of the added interval.

*/
            t = ur.timeInterval.end;

/*
Since we added an interval for both unit interval, increase both unit
pointers.

*/
            if (++mrUnit < mr.GetNoComponents())
                mr.Get(mrUnit, ur);
            if (++mpUnit < mp.GetNoComponents())
                mp.Get(mpUnit, up);
        } else if (ur.timeInterval.Inside(up->timeInterval)) {
/*
\textbf{Case 2:} Interval from ~mr~ is embedded into interval from ~mp~.

*/

            if (MRA_DEBUG) {
                cerr << "RP::RP()   ur:  |---|" << endl;
                cerr << "RP::RP()   up: |-----|" << endl;
            }

/*
Check if we have to add the left side of the interval from ~mp~ to the
refinement partition first.

*/
            if (t < ur.timeInterval.start)
                AddUnits(
                    -1,
                    mpUnit,
                    t > up->timeInterval.start
                    ? t
                    : up->timeInterval.start,
                    ur.timeInterval.start,
                    !c,
                    !ur.timeInterval.lc);

/*
Now add interval from ~mr~. Since it is entirely embedded into the interval
from ~mp~, we directly use its border open/close flags.

*/
            AddUnits(
                mrUnit,
                mpUnit,
                ur.timeInterval.start,
                ur.timeInterval.end,
                ur.timeInterval.lc,
                ur.timeInterval.rc);

/*
Remember right border, right border flag and increase unit pointer ~mrUnit~.

*/
            t = ur.timeInterval.end;
            c = ur.timeInterval.rc;

            if (++mrUnit < mr.GetNoComponents())
                mr.Get(mrUnit, ur);
        } else if (up->timeInterval.Inside(ur.timeInterval)) {
/*
\textbf{Case 3:} Interval from ~mp~ is embedded into interval from ~mr~.
This is similar to case 2 and no further details need to be explained.

*/

            if (MRA_DEBUG) {
                cerr << "RP::RP()   ur: |-----|" << endl;
                cerr << "RP::RP()   up:  |---|" << endl;
            }

            if (t < up->timeInterval.start)
                AddUnits(
                    mrUnit,
                    -1,
                    t > ur.timeInterval.start
                      ? t
                      : ur.timeInterval.start,
                    up->timeInterval.start,
                    !c,
                    !up->timeInterval.lc);

            AddUnits(
                mrUnit,
                mpUnit,
                up->timeInterval.start,
                up->timeInterval.end,
                up->timeInterval.lc,
                up->timeInterval.rc);

            t = up->timeInterval.end;
            c = up->timeInterval.rc;

            if (++mpUnit < mp.GetNoComponents())
                mp.Get(mpUnit, up);
        } else if (ur.timeInterval.Intersects(up->timeInterval)) {
/*
\textbf{Case 4:} The intervals intersect. Further analysis is required,
which will be handled by sub-cases of case 4.

*/
            if (MRA_DEBUG)
                cerr << "RP::RP()   intersect" << endl;

            if (ur.timeInterval.start.Compare(
                    &up->timeInterval.end) == 0
                && ur.timeInterval.lc
                && up->timeInterval.rc) {
/*
\textbf{Case 4.1:} Left border of interval from ~mr~ and right border of
interval from ~mp~ are identical and both intervals are closed in this
point.

*/

                if (MRA_DEBUG) {
                    cerr << "RP::RP()   ur:     [---|" << endl;
                    cerr << "RP::RP()   up: |---]" << endl;
                }

/*
Check if we have to add the interval from ~mp~ minus its right border to the
refinement partition first.

*/
                if (t < up->timeInterval.end)
                    AddUnits(
                        -1,
                        mpUnit,
                        t > up->timeInterval.start
                          ? t
                          : up->timeInterval.start,
                        up->timeInterval.end,
                        !c,
                        false);

/*
Add point interval for the common border.

*/
                AddUnits(
                    mrUnit,
                    mpUnit,
                    ur.timeInterval.start,
                    ur.timeInterval.start,
                    true,
                    true);

/*
Remember point interval border and increase unit pointer ~mpUnit~.

*/
                t = up->timeInterval.start;
                c = true;

                if (++mpUnit < mp.GetNoComponents())
                    mp.Get(mpUnit, up);
            } else if (ur.timeInterval.end.Compare(
                           &up->timeInterval.start) == 0
                       && ur.timeInterval.rc
                       && up->timeInterval.lc) {
/*
\textbf{Case 4.2:} Right border of interval from ~mr~ and left border of
interval from ~mp~ are identical and both intervals are closed in this
point. Similar to case 4.1, no further details explained.

*/
                if (MRA_DEBUG) {
                    cerr << "RP::RP()   ur: |---]" << endl;
                    cerr << "RP::RP()   up:     [---|" << endl;
                }

                if (t < ur.timeInterval.end)
                    AddUnits(
                        mrUnit,
                        -1,
                        t > ur.timeInterval.start
                          ? t
                          : ur.timeInterval.start,
                        ur.timeInterval.end,
                        !c,
                        false);

                AddUnits(
                    mrUnit,
                    mpUnit,
                    up->timeInterval.start,
                    up->timeInterval.start,
                    true,
                    true);

                t = ur.timeInterval.end;
                c = true;

                if (++mrUnit < mr.GetNoComponents())
                    mr.Get(mrUnit, ur);
            } else if (ur.timeInterval.start.Compare(
                           &up->timeInterval.start) < 0) {
/*
\textbf{Case 4.3:} Both intervals overlap, interval from ~mr~ starts right
of interval from ~mp~.

*/

                if (MRA_DEBUG) {
                    cerr << "RP::RP()   ur: |----|" << endl;
                    cerr << "RP::RP()   up:    |----|" << endl;
                }

/*
Check if we have to add the left part of the interval from ~mr~ to the
refinement partition first.

*/
                if (t < up->timeInterval.start)
                    AddUnits(
                        mrUnit,
                        -1,
                        t > ur.timeInterval.start
                          ? t
                          : ur.timeInterval.start,
                        up->timeInterval.start,
                        !c,
                        !up->timeInterval.lc);

/*
Add the intersection of the two intervals by using the respective left or
right borders and their open/close flags.

*/
                AddUnits(
                    mrUnit,
                    mpUnit,
                    up->timeInterval.start,
                    ur.timeInterval.end,
                    up->timeInterval.lc,
                    ur.timeInterval.rc);

/*
Remember interval border and increase unit pointer ~mrUnit~.

*/
                t = ur.timeInterval.end;
                c = ur.timeInterval.rc;

                if (++mrUnit < mr.GetNoComponents())
                    mr.Get(mrUnit, ur);
            } else if (ur.timeInterval.start.Compare(
                           &up->timeInterval.start) == 0) {
/*
\textbf{Case 4.4:} Both intervals have the same start point.

The following assertion holds or we would have cases 2 or 3, which have been
already checked.

*/
                assert(!ur.timeInterval.lc || !up->timeInterval.lc);

/*
Case 4.4 is broken into two sub-cases again.

*/
                if (ur.timeInterval.end.Compare(
                        &up->timeInterval.end) < 0) {
/*
\textbf{Case 4.4.1:} The right border of the interval from ~mr~ is lower
than the right border of the interval from ~mp~.

*/
                    if (MRA_DEBUG) {
                        cerr << "RP::RP()   ur: [---|"
                             << endl;
                        cerr << "RP::RP()   up: ]------|"
                             << endl;
                    }

/*
Add left point of interval from ~mp~ to refinement partition.

*/
                    AddUnits(
                        mrUnit,
                        -1,
                        ur.timeInterval.start,
                        ur.timeInterval.start,
                        true,
                        true);

/*
Add intersection to refinement partition.

*/

                    AddUnits(
                        mrUnit,
                        mpUnit,
                        ur.timeInterval.start,
                        ur.timeInterval.end,
                        false,
                        ur.timeInterval.rc);

/*
Remember interval border and increase unit pointer ~mrUnit~.

*/
                    t = ur.timeInterval.end;
                    c = ur.timeInterval.rc;

                    if (++mrUnit < mr.GetNoComponents())
                        mr.Get(mrUnit, ur);
                } else {
/*
\textbf{Case 4.4.2:} The right border of the interval from ~mr~ is greater
than the right border of the interval from ~mp~.

The following assertion holds or we would have cases 2 or 3, which have been
already checked.

*/
                    assert(ur.timeInterval.end.Compare(
                               &up->timeInterval.end) != 0);

                    if (MRA_DEBUG) {
                        cerr << "RP::RP()   ur: ]------|"
                             << endl;
                        cerr << "RP::RP()   up: [---|"
                             << endl;
                    }

/*
Add left point of interval from ~mr~ to refinement partition.

*/
                    AddUnits(
                        -1,
                        mpUnit,
                        up->timeInterval.start,
                        up->timeInterval.start,
                        true,
                        true);

/*
Add intersection to refinement partition.

*/
                    AddUnits(
                        mrUnit,
                        mpUnit,
                        ur.timeInterval.start,
                        up->timeInterval.end,
                        false,
                        up->timeInterval.rc);

                    t = up->timeInterval.end;
                    c = up->timeInterval.rc;

/*
Remember interval border and increase unit pointer ~mpUnit~.

*/
                    if (++mpUnit < mp.GetNoComponents())
                        mp.Get(mpUnit, up);
                }
            } else {
/*
\textbf{Case 4.5:} Both intervals overlap, interval from ~mr~ starts right
of interval from ~mp~. Similar to case 4.3, no further details explained.

*/

                if (MRA_DEBUG) {
                    cerr << "RP::RP()   ur:    |----|" << endl;
                    cerr << "RP::RP()   up: |----|" << endl;
                }

                if (t < ur.timeInterval.start)
                    AddUnits(
                        -1,
                        mpUnit,
                        t > up->timeInterval.start
                          ? t
                          : up->timeInterval.start,
                        ur.timeInterval.start,
                        !c,
                        !ur.timeInterval.lc);

                AddUnits(
                    mrUnit,
                    mpUnit,
                    ur.timeInterval.start,
                    up->timeInterval.end,
                    ur.timeInterval.lc,
                    up->timeInterval.rc);

                t = up->timeInterval.end;
                c = up->timeInterval.rc;

                if (++mpUnit < mp.GetNoComponents())
                    mp.Get(mpUnit, up);
            }
        } else if (ur.timeInterval.end.Compare(
                       &up->timeInterval.start) <= 0) {
/*
\textbf{Case 5:} Both intervals are disjunct, the interval from ~mr~ is
left of the interval from ~mp~.

*/
            if (MRA_DEBUG) {
                cerr << "RP::RP()   ur: |--|" << endl;
                cerr << "RP::RP()   up:      |--|" << endl;
            }

            AddUnits(
                mrUnit,
                -1,
                t,
                ur.timeInterval.end,
                !c,
                ur.timeInterval.lc);

            t = up->timeInterval.start;
            c = !up->timeInterval.lc;

            if (++mrUnit < mr.GetNoComponents())
                mr.Get(mrUnit, ur);
        } else {
/*
\textbf{Case 6:} Both intervals are disjunct, the interval from ~mr~ is
right of the interval from ~mp~.

*/

            if (MRA_DEBUG) {
                cerr << "RP::RP()   ur:      |--|" << endl;
                cerr << "RP::RP()   up: |--|" << endl;
            }

            AddUnits(
                -1,
                mpUnit,
                t,
                up->timeInterval.end,
                !c,
                up->timeInterval.lc);

            t = ur.timeInterval.start;
            c = !ur.timeInterval.lc;

            if (++mpUnit < mp.GetNoComponents())
                mp.Get(mpUnit, up);
        }
    }

/*
When we reach this point, either all units in ~mr~ or all units in ~mp~
have been examined in the ~while~ loop.

*/

    if (mrUnit < mr.GetNoComponents()) {
/*
There are still units to be processed in ~mr~.

First check whether processing already reached the end of the current unit's
interval. If not, add its right part to the refinement partition.

*/
        if (t < ur.timeInterval.end)
            AddUnits(
                mrUnit,
                -1,
                t,
                ur.timeInterval.end,
                !c,
                ur.timeInterval.rc);
        mrUnit++;

/*
Now just add the remaining unit intervals to the refinement partition.

*/
        while (mrUnit < mr.GetNoComponents()) {
            mr.Get(mrUnit, ur);

            AddUnits(
                mrUnit,
                -1,
                ur.timeInterval.start,
                ur.timeInterval.end,
                ur.timeInterval.lc,
                ur.timeInterval.rc);

            mrUnit++;
        }
    }

/*
There are still units to be processed in ~mp~. See the previous case for
~mr~ for details.

*/
    if (mpUnit < mp.GetNoComponents()) {
        if (t < up->timeInterval.end)
            AddUnits(
                -1,
                mpUnit,
                t,
                up->timeInterval.end,
                !c,
                up->timeInterval.rc);
        mpUnit++;

        while (mpUnit < mp.GetNoComponents()) {
            mp.Get(mpUnit, up);

            AddUnits(
                -1,
                mpUnit,
                up->timeInterval.start,
                up->timeInterval.end,
                up->timeInterval.lc,
                up->timeInterval.rc);

            mpUnit++;
        }
    }
}

/*
1.1.1 Destructor template

*/
template<class Mapping1, class Mapping2, class Unit1, class Unit2>
RefinementPartition<Mapping1, Mapping2, Unit1, Unit2>
::~RefinementPartition() {
    if (MRA_DEBUG)
        cerr << "RP::~RP() called" << endl;

    for (unsigned int i = 0; i < iv.size(); i++) delete iv[i];
}

/*
1.1.1 Method template ~AddUnits()~

*/
template<class Mapping1, class Mapping2, class Unit1, class Unit2>
void RefinementPartition<Mapping1, Mapping2, Unit1, Unit2>
::AddUnits(
    const int urPos,
    const int upPos,
    const Instant& start,
    const Instant& end,
    const bool lc,
    const bool rc) {
    if (MRA_DEBUG) {
        cerr << "RP::AddUnits() called" << endl;
        cerr << "RP::AddUnits() start="
             << start.ToDouble()
             << " end="
             << end.ToDouble()
             << " lc="
             << lc
             << " rc="
             << rc
             << " urPos="
             << urPos
             << " upPos="
             << upPos
             << endl;
    }

    Interval<Instant>* civ =
        new Interval<Instant>(start, end, lc, rc);

    iv.push_back(civ);
    vur.push_back(urPos);
    vup.push_back(upPos);
}

/*
1 Data type ~intimeregion~

The code for this data type is fairly simple because it can rely on the
instantiation of class templates from the ~TemporalAlgebra~.

*/

static ListExpr IRegionProperty() {
    if (MRA_DEBUG) cerr << "IRegionProperty() called" << endl;

    ListExpr example = nl->TextAtom();
    nl->AppendText(example,
                   "(\"2003-01-10\" ((((1.0 3.5)(2.0 5.5)(3.0 6.5)(4.0 6.5)"
                   "(4.0 5.5)(5.0 4.5)(5.0 2.5)(4.0 1.5)(3.0 1.5))"
                   "((2.0 3.0)(2.0 4.0)(3.0 4.0)(3.0 3.0)))))");
    return
        nl->TwoElemList(
            nl->FourElemList(
                nl->StringAtom("Signature"),
                nl->StringAtom("Example Type List"),
                nl->StringAtom("List Rep"),
                nl->StringAtom("Example List")),
            nl->FourElemList(
                nl->StringAtom("-> UNIT"),
                nl->StringAtom("(iregion)"),
                nl->StringAtom("(<instant> <region>)"),
                example));
}

static bool CheckIRegion(ListExpr type, ListExpr& errorInfo) {
    if (MRA_DEBUG) cerr << "CheckIRegion() called" << endl;

    return nl->IsEqual(type, "intimeregion");
}

static TypeConstructor iregion(
    "intimeregion",
    IRegionProperty,
    OutIntime<CRegion, OutRegion>,
    InIntime<CRegion, InRegion>,
    0, 0, // SaveToList, RestoreFromList
    CreateIntime<CRegion>,
    DeleteIntime<CRegion>,
    0, 0, // open, save
    CloseIntime<CRegion>,
    CloneIntime<CRegion>,
    CastIntime<CRegion>,
    SizeOfIntime<CRegion>,
    CheckIRegion );

/*
1 Data type ~uregion~
\label{uregion}

This data type implements regions units.

1.1 Class ~URegion~

1.1.1 Class definition

This extension of ~SpatialTemporalUnit<CRegion, 3>~ is fairly simple in its
structure but offers complex methods.

Since ~URegion~ instances are typically referenced by ~MRegion~ instances,
there is one detail about SECONDO's ~DBArray~ mechanism to be considered.

*/

class URegion : public SpatialTemporalUnit<CRegion, 3> {
private:
/*
A ~URegion~ instance either maintains its own memory for segments
(~role = NORMAL~) or receives it from its ~MRegion~ instance
(~role = EMBEDDED~).

*/
    enum { NORMAL, EMBEDDED } role;

/*
~segments~ is a pointer to a DBArray, either allocated by the ~URegion~
instance itself or passed by its ~MRegion~ instance. See ~type~ to find
out which is the current case.

~segmentsStartPos~ is the index in ~segments~, where this instances
segments are starting. If the instance allocated its own memory,
~segmentsStartPos~ is $0$.

~segmentsNum~ is the number of segments in the ~URegion~ instance.

*/
    DBArray<MSegmentData>* segments;
    unsigned int segmentsStartPos;
    unsigned int segmentsNum;

/*
Return the number of segments in ~segments~, which is locate above
~up~ during the interval ~iv~. Before calling this method, it must be
assured that the point does not intersect with one of the segments
during the interval.

*/
    unsigned int Plumbline(const UPoint& up, const Interval<Instant>& iv) const;

/*
Add new ~UPoint~ unit to ~res~ which reflects that during the
interface $(starttime, endtime, lc, rc)$ the moving point was
from coordinates $(x0, y0)$ to $(x1, y1)$ within this ~URegion~
instance. ~pending~ is used to merge ~UPoint~ instances, if possible.

*/

    void RestrictedIntersectionAddUPoint(MPoint& res,
                                         double starttime,
                                         double endtime,
                                         bool lc,
                                         bool rc,
                                         double x0,
                                         double y0,
                                         double x1,
                                         double y1,
                                         UPoint*& pending) const;

/*
Collect 'normal' intersections between ~UPoint~ unit ~rUp~ and moving segment
~rDms~, which occured in interval ~iv~ at point $(ip1x, ip1y)$ at the time
$ip1t$. The intersections are written to the vector ~vtsi~.

*/
    void RestrictedIntersectionFindNormal(
        const Interval<Instant>& iv,
        const UPoint& rUp,
        MSegmentData& rDms,
        bool& ip1present,
        double& ip1x,
        double& ip1y,
        double& ip1t,
        vector<TrapeziumSegmentIntersection>& vtsi) const;

/*
Handle intersection line between ~UPoint~ unit ~rUp~ and a moving segment,
which lies in the same plane as the moving segment and occured in the
intervall ~iv~. $(ip1x, ip1y)$ is the
first intersection point, $(ip2y, ip2y)$ is the second intersection point,
~ip1present~ and ~ip2present~ denote whether both points are present
(they may be reduced to a single point) The intersections are written to
the vector ~vtsi~.

*/
    void RestrictedIntersectionFindInPlane(
        const Interval<Instant>& iv,
        const UPoint& rUp,
        bool& ip1present,
        double& ip1x,
        double& ip1y,
        double& ip1t,
        bool& ip2present,
        double& ip2x,
        double& ip2y,
        double& ip2t,
        vector<TrapeziumSegmentIntersection>& vtsi) const;

/*
Find all intersections between the ~UPoint~ unit ~up~ and the segments
of this ~URegion~ instance during the interval ~iv~. Write intersections
to vector ~vtsi~.

*/
    void RestrictedIntersectionFind(
        const UPoint& up,
        const Interval<Instant>& iv,
        vector<TrapeziumSegmentIntersection>& vtsi) const;

/*
For the intersections between ~up~ and this ~URegion~ instance's segments
stored in ~vtsi~,
match two pairs of intersection points, which have been previously calculated
by ~RestrictedIntersectionFind()~, so that each pair contains an intersection
point where the ~URegion~ instance is being entered and an intersection point
where it is left. From each pair, construct a ~UPoint~ unit and add it to
~res~. ~pending~ is used to merge ~UPoint~ instances, if possible.

*/
    bool RestrictedIntersectionProcess(
        const UPoint& up,
        const Interval<Instant>& iv,
        vector<TrapeziumSegmentIntersection>& vtsi,
        MPoint& res,
        UPoint*& pending) const;

/*
Handle intersections in ~vtsi~ with degenerated segments.

*/
    void RestrictedIntersectionFix(
        vector<TrapeziumSegmentIntersection>& vtsi) const;

public:
    URegion() {
        if (MRA_DEBUG)
            cerr << "URegion::URegion() #1 called"
                 << endl;
    }

/*
Constructor, which received pointer to storage for segments from
~MRegion~ instance.

*/
    URegion(const Interval<Instant>& interval,
            DBArray<MSegmentData>* segs,
            unsigned int pos) :
        SpatialTemporalUnit<CRegion, 3>(interval),
        role(EMBEDDED),
        segments(segs),
        segmentsStartPos(pos),
        segmentsNum(0) {

        if (MRA_DEBUG)
            cerr << "URegion::URegion() #2 called"
                 << endl;
    }

/*
Constructor, which allocates its own storage for segments..

*/
    URegion(const Interval<Instant>& interval) :
        SpatialTemporalUnit<CRegion, 3>(interval),
        role(NORMAL),
        segments(new DBArray<MSegmentData>(0)),
        segmentsStartPos(0),
        segmentsNum(0) {

        if (MRA_DEBUG)
            cerr << "URegion::URegion() #3 called"
                 << endl;

/*
Lets see if this constructor is really required. If somebody complains,
we know it. That's solid empirical reasoning!

*/
        assert(false);
    }

/*
Constructor, which receives storage for its segments from ~MRegion~
instance and creates a constant unit for the specied interval from the
provided ~CRegion~ instance.

*/
    URegion(const Interval<Instant>& interval,
            const CRegion& region,
            DBArray<MSegmentData>* segs,
            unsigned int pos) :
        SpatialTemporalUnit<CRegion, 3>(interval),
        role(EMBEDDED),
        segments(segs),
        segmentsStartPos(pos),
        segmentsNum(0) {

        if (MRA_DEBUG)
            cerr << "URegion::URegion() #4 called" << endl;

        for (int i = 0; i < region.Size(); i += 2) {
            const CHalfSegment *thisChs;
            region.Get(i, thisChs);

            const CHalfSegment *nextChs;
            region.Get(i+1 == region.Size() ? 0 : i+1, nextChs);

            MSegmentData dms(thisChs->GetAttr().faceno,
                             thisChs->GetAttr().cycleno,
                             thisChs->GetAttr().edgeno,
                             thisChs->GetAttr().insideAbove,
                             thisChs->GetLP().GetX(),
                             thisChs->GetLP().GetY(),
                             thisChs->GetRP().GetX(),
                             thisChs->GetRP().GetY(),
                             thisChs->GetLP().GetX(),
                             thisChs->GetLP().GetY(),
                             thisChs->GetRP().GetX(),
                             thisChs->GetRP().GetY());

            dms.degeneratedInitial = DGM_NONE;
            dms.degeneratedFinal = DGM_NONE;

            segments->Resize(segmentsStartPos+segmentsNum+1);
            segments->Put(segmentsStartPos+segmentsNum, dms);
            segmentsNum++;
        }
    }

/*
Required for ~OpenMRegion()~. Set the storage for segments after the
instance has been created.

*/
    void SetMSegmentData(DBArray<MSegmentData>* s) {
        if (MRA_DEBUG)
            cerr << "URegion::SetMSegmentData() called" << endl;

        segments = s;
    }

/*
Adds a segment to this ~URegion~ instance.

~cr~ is a ~CRegion~ instance, which is used to check whether the ~URegion~
instance represents a valid region in the middle of its interval, when no
degeneration can occur. All segments added to the ~URegion~ instance are
added as non-moving segments to ~cr~ too.

~rDir~ is used to check the direction of a cylce, which is required to
finalise the ~insideAbove~ attributes of the segments.

Both ~cr~ and ~rDir~ are checked outside the methods of ~URegion~, they
are just filled by ~AddSegment()~.

~faceno~, ~cycleno~, ~segmentno~ and ~pointno~ are the obvious indices
in the ~URegion~ unit's segment.

~intervalLen~ is the length of the interval covered by this ~URegion~
instance.

~start~ and ~end~ contain the initial and final positions of the segment
in list representation, which will be added to the ~URegion~ instance

~AddSegment()~ performs numerous calculations and checks. See the method
description below for details.

*/
    bool AddSegment(CRegion& cr,
                    CRegion& rDir,
                    unsigned int faceno,
                    unsigned int cycleno,
                    unsigned int segmentno,
                    unsigned int pointno,
                    double intervalLen,
                    ListExpr start,
                    ListExpr end);

/*
Set the value of the ~insideAbove~ attribut of segment as position ~pos~ to
the value ~insideAbove~.

*/

    void SetSegmentInsideAbove(int pos, bool insideAbove);

/*
Get number of segments, get specific segment, write specific segment.

*/
    int GetSegmentsNum(void) const;
    void GetSegment(int pos, const MSegmentData*& dms) const;
    void PutSegment(int pos, const MSegmentData& dms);

/*
Calculate ~MPoint~ instance ~res~ from the intersection ~up~ and this
~URegion~ unit, restricted to interval ~iv~ instead of ~up~'s or this
instances full intervals. ~pending~ is used to try to merge multiple
units.

*/
    void RestrictedIntersection(const UPoint& up,
                                const Interval<Instant>& iv,
                                MPoint& res,
                                UPoint*& pending) const;

/*
Destroy the ~URegion~ unit. If it has its own segments memory, free it.

*/
    void Destroy(void) {
        if (MRA_DEBUG) cerr << "URegion::Destroy() called" << endl;

        assert(role == NORMAL || role == EMBEDDED);

        if (role == NORMAL) segments->Destroy();
    }

/*
Returns the ~CRegion~ value of this ~URegion~ unit at instant ~t~
in ~result~.

*/
    virtual void TemporalFunction(const Instant& t, CRegion& result) const;

/*
~At()~, ~Passes()~ and ~BoundingBox()~ are not yet implemented. Stubs
required for to make this class non-abstract.

*/
    virtual bool At(const CRegion& val, TemporalUnit<CRegion>& result) const;
    virtual bool Passes(const CRegion& val) const;

    virtual const Rectangle<3> BoundingBox() const;

/*
Required for Algebra integration. Not implemented either.

*/
    virtual URegion* Clone() const;
    virtual void CopyFrom(const StandardAttribute* right);
};

/*
1.1.1 Method stubs

*/
URegion* URegion::Clone(void) const {
    if (MRA_DEBUG) cerr << "URegion::Clone() called" << endl;

    assert(false);
}

void URegion::CopyFrom(const StandardAttribute* right) {
    if (MRA_DEBUG) cerr << "URegion::CopyFrom() called" << endl;

    assert(false);
}

const Rectangle<3> URegion::BoundingBox() const {
    if (MRA_DEBUG) cerr << "URegion::BoundingBox() called" << endl;

    assert(false);
}

/*
1.1.1 Method ~At()~

*/
bool URegion::At(const CRegion& val, TemporalUnit<CRegion>& result) const {
    if (MRA_DEBUG) cerr << "URegion::At() called" << endl;

    assert(false);
}

/*
1.1.1 Method ~Passes()~

*/
bool URegion::Passes(const CRegion& val) const {
    if (MRA_DEBUG) cerr << "URegion::At() called" << endl;

    assert(false);
}

/*
1.1.1 Methods required for intersection with ~UPoint~

1.1.1.1 Method ~RestrictedIntersectionFindNormal()~

*/
void URegion::RestrictedIntersectionFindNormal(
    const Interval<Instant>& iv,
    const UPoint& rUp,
    MSegmentData& rDms,
    bool& ip1present,
    double& ip1x,
    double& ip1y,
    double& ip1t,
    vector<TrapeziumSegmentIntersection>& vtsi) const {

    if (MRA_DEBUG)
        cerr << "URegion::RIFN() called" << endl;

/*
Calculate end points of moving segment at time of intersection ~ip1t~.

*/
    double p1x, p1y, p2x, p2y;

    if (nearlyEqual(iv.start.ToDouble(), iv.end.ToDouble())) {
        p1x = rDms.initialStartX;
        p1y = rDms.initialStartY;
        p2x = rDms.initialEndX;
        p2y = rDms.initialEndY;
    } else {
        double f =
            (ip1t-iv.start.ToDouble())
            /(iv.end.ToDouble()-iv.start.ToDouble());

        p1x =
            rDms.initialStartX
            +(rDms.finalStartX-rDms.initialStartX)*f;
        p1y =
            rDms.initialStartY
            +(rDms.finalStartY-rDms.initialStartY)*f;
        p2x =
            rDms.initialEndX
            +(rDms.finalEndX-rDms.initialEndX)*f;
        p2y =
            rDms.initialEndY
            +(rDms.finalEndY-rDms.initialEndY)*f;
    }

    if (MRA_DEBUG)
        cerr << "URegion::RIFN() p1=("
             << p1x << " " << p1y
             << ") p2=("
             << p2x << " " << p2y
             << ")"
             << " ip1=("
             << ip1x << " " << ip1y << " " << ip1t << ")"
             << endl;

    TrapeziumSegmentIntersection tsi;

    tsi.x = ip1x;
    tsi.y = ip1y;
    tsi.t = ip1t;

/*
Is the ~UPoint~ instance constant and is it located on the segment?

*/
    if (nearlyEqual(rUp.p0.GetX(), ip1x)
        && nearlyEqual(rUp.p0.GetY(), ip1y)
        && nearlyEqual(rUp.p1.GetX(), ip1x)
        && nearlyEqual(rUp.p1.GetY(), ip1y)) {
        if (MRA_DEBUG)
            cerr << "URegion::RIFN() through p0 and p1" << endl;

/*
Yes, lets see if we the moving segment did not touch the ~UPoint~ instance
during initial time. If yes, we can deduce from the ~insideAbove~ attribute
and the direction of the movement whether the ~URegion~ has been entered or
left at the intersection point. If not, we just two identical points, one
with TSI\_ENTER and one with TSI\_LEAVE, which indicates that the ~URegion~
has been entered and left in the same instance.

*/
        if (!nearlyEqual(ip1t, iv.start.ToDouble())) {
            if (pointAboveSegment(ip1x, ip1y,
                                  rDms.initialStartX, rDms.initialStartY,
                                  rDms.initialEndX, rDms.initialEndY)) {
                tsi.type = rDms.insideAbove ? TSI_LEAVE : TSI_ENTER;
            } else {
                tsi.type = rDms.insideAbove ? TSI_ENTER : TSI_LEAVE;
            }

            if (MRA_DEBUG)
                cerr << "URegion::RIFN() adding "
                     << tsi.type
                     << " " << tsi.x << " " << tsi.y << " " << tsi.t
                     << endl;
            vtsi.push_back(tsi);
        } else if (!nearlyEqual(ip1t, iv.end.ToDouble())) {
            if (pointAboveSegment(ip1x, ip1y,
                                  rDms.finalStartX, rDms.finalStartY,
                                  rDms.finalEndX, rDms.finalEndY)) {
                tsi.type = rDms.insideAbove ? TSI_ENTER : TSI_LEAVE;
            } else {
                tsi.type = rDms.insideAbove ? TSI_LEAVE : TSI_ENTER;
            }

            if (MRA_DEBUG)
                cerr << "URegion::RIFN() adding "
                     << tsi.type
                     << " " << tsi.x << " " << tsi.y << " " << tsi.t
                     << endl;
            vtsi.push_back(tsi);
        } else {
            if (MRA_DEBUG)
                cerr << "URegion::RIFN() adding "
                     << tsi.type
                     << " " << tsi.x << " " << tsi.y << " " << tsi.t
                     << endl;
            tsi.type = TSI_ENTER;
            vtsi.push_back(tsi);

            if (MRA_DEBUG)
                cerr << "URegion::RIFN() adding "
                     << tsi.type
                     << " " << tsi.x << " " << tsi.y << " " << tsi.t
                     << endl;
            tsi.type = TSI_LEAVE;
            vtsi.push_back(tsi);
        }

/*
Is one of the endpoints of the ~UPoint~ instance located on the segment?
If so, check whether the other endpoint is above or below the segment.
Together with the value of the ~insideAbove~ attribute, we can deduce
whether the ~URegion~ instance has been entered or left.

The same is checked for the end of the interval. This is to determine
whether point and segment move into the same direction with the segment
being faster than the point. In this case, both only touch without the
point actually entering the ~URegion~ instance. You can visualise this
by imagining that the ~segment~ is overtaking the point.

*/
    } else if (nearlyEqual(rUp.p0.GetX(), ip1x)
               && nearlyEqual(rUp.p0.GetY(), ip1y)) {
        if (MRA_DEBUG)
            cerr << "URegion::RIFN() through p0" << endl;

        if (pointAboveSegment(rUp.p1.GetX(), rUp.p1.GetY(),
                              p1x, p1y, p2x, p2y)) {
            tsi.type = rDms.insideAbove ? TSI_ENTER : TSI_LEAVE;
        } else {
            tsi.type = rDms.insideAbove ? TSI_LEAVE : TSI_ENTER;
        }

        if (MRA_DEBUG)
            cerr << "URegion::RIFN() adding "
                 << tsi.type
                 << " " << tsi.x << " " << tsi.y << " " << tsi.t
                 << endl;

        vtsi.push_back(tsi);

        if (pointAboveSegment(rUp.p1.GetX(), rUp.p1.GetY(),
                              rDms.finalStartX, rDms.finalStartY,
                              rDms.finalEndX, rDms.finalEndY)) {
            tsi.type = tsi.type == TSI_ENTER ? TSI_LEAVE : TSI_ENTER;

            if (MRA_DEBUG)
                cerr << "URegion::RIFN() adding "
                     << tsi.type
                     << " " << tsi.x << " " << tsi.y << " " << tsi.t
                     << endl;

            vtsi.push_back(tsi);
        }

/*
Is one of the endpoints of the ~UPoint~ instance located on the segment?
If so, see handling of previous case.

*/

    } else if (nearlyEqual(rUp.p1.GetX(), ip1x)
               && nearlyEqual(rUp.p1.GetY(), ip1y)) {
        if (MRA_DEBUG)
            cerr << "URegion::RIFN() through p1" << endl;

        if (pointAboveSegment(rUp.p0.GetX(), rUp.p0.GetY(),
                              p1x, p1y, p2x, p2y)) {
            tsi.type = rDms.insideAbove ? TSI_LEAVE : TSI_ENTER;
        } else {
            tsi.type = rDms.insideAbove ? TSI_ENTER : TSI_LEAVE;
        }

        if (MRA_DEBUG)
            cerr << "URegion::RIFN() adding "
                 << tsi.type
                 << " " << tsi.x << " " << tsi.y << " " << tsi.t
                 << endl;

        vtsi.push_back(tsi);

        if (pointAboveSegment(rUp.p0.GetX(), rUp.p0.GetY(),
                              rDms.initialStartX, rDms.initialStartY,
                              rDms.initialEndX, rDms.initialEndY)) {
            tsi.type = tsi.type == TSI_ENTER ? TSI_LEAVE : TSI_ENTER;

            if (MRA_DEBUG)
                cerr << "URegion::RIFN() adding "
                     << tsi.type
                     << " " << tsi.x << " " << tsi.y << " " << tsi.t
                     << endl;

            vtsi.push_back(tsi);
        }

/*
Is one of the endpoints of the ~UPoint~ instance located above the
segment? Determining whether the ~UPoint~ instance entered the
~URegion~ instance is simple then and relies on the ~insideAbove~
attribute again. Moreover, it needs to check the same ~overtaking~
situation as described for the previous cases before.

*/
    } else if (pointAboveSegment(rUp.p0.GetX(), rUp.p0.GetY(),
                                 p1x, p1y, p2x, p2y)) {
        if (MRA_DEBUG)
            cerr << "URegion::RIFN() p0 above segment" << endl;

        tsi.type = rDms.insideAbove ? TSI_LEAVE : TSI_ENTER;

        if (MRA_DEBUG)
            cerr << "URegion::RIFN() adding "
                 << tsi.type
                 << " " << tsi.x << " " << tsi.y << " " << tsi.t
                 << endl;

        vtsi.push_back(tsi);

        if (pointAboveSegment(rUp.p1.GetX(), rUp.p1.GetY(),
                              rDms.finalStartX, rDms.finalStartY,
                              rDms.finalEndX, rDms.finalEndY)) {
            tsi.type = rDms.insideAbove ? TSI_ENTER : TSI_LEAVE;

            if (MRA_DEBUG)
                cerr << "URegion::RIFN() adding "
                     << tsi.type
                     << " " << tsi.x << " " << tsi.y << " " << tsi.t
                     << endl;

            vtsi.push_back(tsi);
        }

/*
In this case, the previously checked endpoint is below the segment.
See handling of previous case.

*/
    } else {
        if (MRA_DEBUG)
            cerr << "URegion::RIFN() p0 below segment" << endl;

        tsi.type = rDms.insideAbove ? TSI_ENTER : TSI_LEAVE;

        if (MRA_DEBUG)
            cerr << "URegion::RIFN() adding "
                 << tsi.type
                 << " " << tsi.x << " " << tsi.y << " " << tsi.t
                 << endl;

        vtsi.push_back(tsi);

        if (pointBelowSegment(rUp.p1.GetX(), rUp.p1.GetY(),
                              rDms.finalStartX, rDms.finalStartY,
                              rDms.finalEndX, rDms.finalEndY)) {
            tsi.type = rDms.insideAbove ? TSI_LEAVE : TSI_ENTER;

            if (MRA_DEBUG)
                cerr << "URegion::RIFN() adding "
                     << tsi.type
                     << " " << tsi.x << " " << tsi.y << " " << tsi.t
                     << endl;

            vtsi.push_back(tsi);
        }
    }
}


/*
1.1.1.1 Method ~RestrictedIntersectionFindInPlane()~

*/
void URegion::RestrictedIntersectionFindInPlane(
    const Interval<Instant>& iv,
    const UPoint& rUp,
    bool& ip1present,
    double& ip1x,
    double& ip1y,
    double& ip1t,
    bool& ip2present,
    double& ip2x,
    double& ip2y,
    double& ip2t,
    vector<TrapeziumSegmentIntersection>& vtsi) const {

    if (MRA_DEBUG)
        cerr << "URegion::RIFIP() called" << endl;

    TrapeziumSegmentIntersection tsi;

    tsi.x = ip1x;
    tsi.y = ip1y;
    tsi.t = ip1t;

    if ((nearlyEqual(ip1x, rUp.p0.GetX())
         && nearlyEqual(ip1y, rUp.p0.GetY())
         && nearlyEqual(ip2x, rUp.p1.GetX())
         && nearlyEqual(ip2y, rUp.p1.GetY()))
        || (nearlyEqual(ip1x, rUp.p1.GetX())
            && nearlyEqual(ip1y, rUp.p1.GetY())
            && nearlyEqual(ip2x, rUp.p0.GetX())
            && nearlyEqual(ip2y, rUp.p0.GetY()))) {
/*
The ~URegion~ happily walks entirely along one of the moving segments of the
~URegion~ instance. When the segments is entered, the ~URegion~ instance
is entered, and when the segment is left, the ~URegion~ instance is left.

*/
        if (MRA_DEBUG)
            cerr << "URegion::RIFIP() up on segment"
                 << endl;

        tsi.type = ip1t < ip2t ? TSI_ENTER : TSI_LEAVE;

        if (MRA_DEBUG)
            cerr << "URegion::RIFIP() adding "
                 << tsi.type
                 << " " << tsi.x << " " << tsi.y << " " << tsi.t
                 << endl;

        vtsi.push_back(tsi);

        tsi.type = ip1t < ip2t ? TSI_LEAVE : TSI_ENTER;
        tsi.x = ip2x;
        tsi.y = ip2y;
        tsi.t = ip2t;

        if (MRA_DEBUG)
            cerr << "URegion::RIFIP() adding "
                 << tsi.type
                 << " " << tsi.x << " " << tsi.y << " " << tsi.t
                 << endl;

        vtsi.push_back(tsi);

    } else if (nearlyEqual(ip1x, rUp.p0.GetX())
               && nearlyEqual(ip1y, rUp.p0.GetY())) {
/*
The ~URegion~ instance is entered via one of its edge points.

*/
        if (MRA_DEBUG)
            cerr << "URegion::RIFIP() ip1=p0" << endl;

        tsi.type = TSI_ENTER;

        if (MRA_DEBUG)
            cerr << "URegion::RIFIP() adding "
                 << tsi.type
                 << " " << tsi.x << " " << tsi.y << " " << tsi.t
                 << endl;

        vtsi.push_back(tsi);

        tsi.type = TSI_LEAVE;
        tsi.x = ip2x;
        tsi.y = ip2y;
        tsi.t = ip2t;

        if (MRA_DEBUG)
            cerr << "URegion::RIFIP() adding "
                 << tsi.type
                 << " " << tsi.x << " " << tsi.y << " " << tsi.t
                 << endl;

        vtsi.push_back(tsi);
    } else if (nearlyEqual(ip1x, rUp.p1.GetX())
               && nearlyEqual(ip1y, rUp.p1.GetY())) {
/*
The ~URegion~ instance is left via one of its edge points. See previous
case.

*/
        if (MRA_DEBUG)
            cerr << "URegion::RIFIP() ip1=p1" << endl;

        tsi.type = TSI_LEAVE;

        if (MRA_DEBUG)
            cerr << "URegion::RIFIP() adding "
                 << tsi.type
                 << " " << tsi.x << " " << tsi.y << " " << tsi.t
                 << endl;

        vtsi.push_back(tsi);

        tsi.type = TSI_ENTER;
        tsi.x = ip2x;
        tsi.y = ip2y;
        tsi.t = ip2t;

        if (MRA_DEBUG)
            cerr << "URegion::RIFIP() adding "
                 << tsi.type
                 << " " << tsi.x << " " << tsi.y << " " << tsi.t
                 << endl;

        vtsi.push_back(tsi);
    } else if (nearlyEqual(ip2x, rUp.p0.GetX())
               && nearlyEqual(ip2y, rUp.p0.GetY())) {
/*
The ~URegion~ instance is entered via one of its edge points. See previous
case.

*/
        if (MRA_DEBUG)
            cerr << "URegion::RIFIP() ip2=p0" << endl;

        tsi.type = TSI_LEAVE;

        if (MRA_DEBUG)
            cerr << "URegion::RIFIP() adding "
                 << tsi.type
                 << " " << tsi.x << " " << tsi.y << " " << tsi.t
                 << endl;

        vtsi.push_back(tsi);

        tsi.type = TSI_ENTER;
        tsi.x = ip2x;
        tsi.y = ip2y;
        tsi.t = ip2t;

        if (MRA_DEBUG)
            cerr << "URegion::RIFIP() adding "
                 << tsi.type
                 << " " << tsi.x << " " << tsi.y << " " << tsi.t
                 << endl;

        vtsi.push_back(tsi);
    } else if (nearlyEqual(ip2x, rUp.p1.GetX())
               && nearlyEqual(ip2y, rUp.p1.GetY())) {
/*
The ~URegion~ instance is left via one of its edge points. See previous
case.

*/
        if (MRA_DEBUG)
            cerr << "URegion::RIFIP() ip2=p1" << endl;

        tsi.type = TSI_ENTER;

        if (MRA_DEBUG)
            cerr << "URegion::RIFIP() adding "
                 << tsi.type
                 << " " << tsi.x << " " << tsi.y << " " << tsi.t
                 << endl;

        vtsi.push_back(tsi);

        tsi.type = TSI_LEAVE;
        tsi.x = ip2x;
        tsi.y = ip2y;
        tsi.t = ip2t;

        if (MRA_DEBUG)
            cerr << "URegion::RIFIP() adding "
                 << tsi.type
                 << " " << tsi.x << " " << tsi.y << " " << tsi.t
                 << endl;

        vtsi.push_back(tsi);
    } else {
/*
The ~URegion~ instance is entered and left elsewhere, ie. in the middle
of the segments.

*/
        if (MRA_DEBUG)
            cerr << "URegion::RIFIP() up covers segment" << endl;

        tsi.type = ip1t < ip2t ? TSI_ENTER : TSI_LEAVE;

        if (MRA_DEBUG)
            cerr << "URegion::RIFIP() adding "
                 << tsi.type
                 << " " << tsi.x << " " << tsi.y << " " << tsi.t
                 << endl;

        vtsi.push_back(tsi);

        tsi.type = ip1t < ip2t ? TSI_LEAVE : TSI_ENTER;
        tsi.x = ip2x;
        tsi.y = ip2y;
        tsi.t = ip2t;

        if (MRA_DEBUG)
            cerr << "URegion::RIFIP() adding "
                 << tsi.type
                 << " " << tsi.x << " " << tsi.y << " " << tsi.t
                 << endl;

        vtsi.push_back(tsi);
    }
}

/*
1.1.1 Method ~Plumbline()~

*/
unsigned int 
URegion::Plumbline(const UPoint& up, const Interval<Instant>& iv) const{
    if (MRA_DEBUG) cerr << "URegion::Plumbline() called" << endl;

/*
Since we know that ~up~ is not intersecting with any of the segments, we
can pick any time in ~iv~ to examine ~up~ and the segments. We pick the
middle of the interval:

*/
    double t =
        iv.start.ToDouble()
        +(iv.end.ToDouble()-iv.start.ToDouble())/2;

    if (MRA_DEBUG)
        cerr << "URegion::Plumbline() iv.start="
             << iv.start.ToDouble()
             << " iv.end="
             << iv.end.ToDouble()
             << " t="
             << t
             << endl;

    double f;

/*
Calculate the position ~x~, ~y~ of ~up~ for ~t~.

*/
    if (nearlyEqual(up.timeInterval.start.ToDouble(),
                    up.timeInterval.end.ToDouble()))
        f = 0;
    else
        f = (t-up.timeInterval.start.ToDouble())
            /(up.timeInterval.end.ToDouble()
              -up.timeInterval.start.ToDouble());

    if (MRA_DEBUG)
        cerr << "URegion::Plumbline() f=" << f << endl;

    if (MRA_DEBUG)
        cerr << "URegion::Plumbline() p0=("
             << up.p0.GetX() << " " << up.p0.GetY()
             << ") p1=("
             << up.p1.GetX() << " " << up.p1.GetY()
             << ")"
             << endl;

    double x = up.p0.GetX()+(up.p1.GetX()-up.p0.GetX())*f;
    double y = up.p0.GetY()+(up.p1.GetY()-up.p0.GetY())*f;

    if (MRA_DEBUG)
        cerr << "URegion::Plumbline() x=" << x << " y=" << y << endl;

/*
Required to calculate the position of each segment for ~t~.

*/
    if (nearlyEqual(timeInterval.start.ToDouble(),
                    timeInterval.end.ToDouble()))
        f = 0;
    else
        f = (t-timeInterval.start.ToDouble())
            /(timeInterval.end.ToDouble()
              -timeInterval.start.ToDouble());

/*
~num~ is used to count the segments above ~(x, y)~.

*/
    unsigned int num = 0;

/*
Go through all segments...

*/
    for (unsigned int i = 0; i < segmentsNum; i++) {
        if (MRA_DEBUG)
            cerr << "URegion::Plumbline() segment #" << i << endl;

        const MSegmentData *dms;
        GetSegment(i, dms);

/*
Calculate position of segment at ~t~.

*/
        double p1x =
            dms->initialStartX
            +(dms->finalStartX-dms->initialStartX)*f;
        double p1y =
            dms->initialStartY
            +(dms->finalStartY-dms->initialStartY)*f;
        double p2x =
            dms->initialEndX
            +(dms->finalEndX-dms->initialEndX)*f;
        double p2y =
            dms->initialEndY
            +(dms->finalEndY-dms->initialEndY)*f;

        if (MRA_DEBUG)
            cerr << "URegion::Plumbline() p1x=" << p1x
                 << " p1y=" << p1y
                 << " p2x=" << p2x
                 << " p2y=" << p2y
                 << endl;

        if (nearlyEqual(p1x, p2x)) {
/*
Ignore vertical segments.

*/
            if (MRA_DEBUG)
                cerr << "URegion::Plumbline() "
                     << "vertical, ignored"
                     << endl;
        } else if (nearlyEqual(p1x, x) && lowerOrNearlyEqual(y, p1y)) {
/*
~(x, y)~ is one of the endpoints. Only count it if it is the right endpoint.

*/
            if (MRA_DEBUG)
                cerr << "URegion::Plumbline() "
                     << "plumbline through start point"
                     << endl;

            if (p1x > p2x) {
                if (MRA_DEBUG)
                    cerr << "URegion::Plumbline() counted" << endl;

                num++;
            } else {
                if (MRA_DEBUG)
                    cerr << "URegion::Plumbline() ignored" << endl;
            }
        } else if (nearlyEqual(p2x, x) && lowerOrNearlyEqual(y, p2y)) {
/*
~(x, y)~ is one of the endpoints. Only count it if it is the right endpoint.

*/
            if (MRA_DEBUG)
                cerr << "URegion::Plumbline() "
                     << "plumbline through end point"
                     << endl;

            if (p1x < p2x) {
                if (MRA_DEBUG)
                    cerr << "URegion::Plumbline() counted" << endl;

                num++;
            } else {
                if (MRA_DEBUG)
                    cerr << "URegion::Plumbline() ignored" << endl;
            }
        } else if (!between(p1x, x, p2x)) {
/*
~(x, y)~ is not below the segment.

*/
            if (MRA_DEBUG)
                cerr << "URegion::Plumbline() "
                     << "x not between p1x, p1y, ignored"
                     << endl;
        } else {
/*
~(x, y)~ is in the same ~x~-range as the segment. Count it if the segment
is above ~(x, y)~.

*/
            if (MRA_DEBUG)
                cerr << "URegion::Plumbline() "
                     << "checking for intersection"
                     << endl;

            if (MRA_DEBUG) {
                cerr << "URegion::Plumbline() nearlyEqual(p1x, x)="
                     << nearlyEqual(p1x, x)
                     << endl;
                cerr << "URegion::Plumbline() nearlyEqual(p2x, x)="
                     << nearlyEqual(p2x, x)
                     << endl;
            }

            // solve p1x+(p2x-p1x)*s = x

            double s = (x-p1x)/(p2x-p1x);
            double ys = p1y+(p2y-p1y)*s;

            if (MRA_DEBUG)
                cerr << "URegion::Plumbline() ys=" << ys << endl;

            if (lowerOrNearlyEqual(y, ys)) {
                if (MRA_DEBUG)
                    cerr << "URegion::Plumbline() below" << endl;

                num++;
            } else {
                if (MRA_DEBUG)
                    cerr << "URegion::Plumbline() not below" << endl;
            }
        }

        if (MRA_DEBUG)
            cerr << "URegion::Plumbline() num=" << num << endl;
    }

    return num;
}

/*
1.1.1 Function ~RestrictedIntersectionFind()~

*/
void URegion::RestrictedIntersectionFind(
    const UPoint& up,
    const Interval<Instant>& iv,
    vector<TrapeziumSegmentIntersection>& vtsi) const {

/*
Straightforward. Even being slightly cryptic, the debug output provides
sufficient context to understand this method.

*/
    if (MRA_DEBUG)
        cerr << "URegion::RIF() called" << endl;

    UPoint rUp;
    restrictUPointToInterval(up, iv, rUp);

    for (unsigned int i = 0; i < segmentsNum; i++) {
        const MSegmentData *dms;
        GetSegment(i, dms);

        if (MRA_DEBUG) {
            cerr << "URegion::RIF() segment #"
                 << i
                 << ": ["
                 << timeInterval.start.ToDouble()
                 << " "
                 << timeInterval.end.ToDouble()
                 << " "
                 << timeInterval.lc << " " << timeInterval.rc
                 << "] ("
                 << dms->initialStartX << " " << dms->initialStartY
                 << " "
                 << dms->initialEndX << " " << dms->initialEndY
                 << ")-("
                 << dms->finalStartX << " " << dms->finalStartY
                 << " "
                 << dms->finalEndX << " " << dms->finalEndY
                 << ") ia="
                 << dms->insideAbove
                 << endl;
            cerr << "URegion::RIF() point is ["
                 << up.timeInterval.start.ToDouble()
                 << " "
                 << up.timeInterval.end.ToDouble()
                 << " "
                 << up.timeInterval.lc << " " << up.timeInterval.rc
                 << "] ("
                 << up.p0.GetX() << " " << up.p0.GetY()
                 << " "
                 << up.p1.GetX() << " " << up.p1.GetY()
                 << ")"
                 << endl;
            cerr << "URegion::RIF() iv is ["
                 << iv.start.ToDouble()
                 << " "
                 << iv.end.ToDouble()
                 << " "
                 << iv.lc
                 << " "
                 << iv.rc
                 << "]"
                 << endl;
        }

        MSegmentData rDms;
        dms->restrictToInterval(timeInterval, iv, rDms);

        if (MRA_DEBUG) {
            cerr << "URegion::RIF() segment restricted to iv is ("
                 << rDms.initialStartX << " " << rDms.initialStartY
                 << " "
                 << rDms.initialEndX << " " << rDms.initialEndY
                 << ")-("
                 << rDms.finalStartX << " " << rDms.finalStartY
                 << " "
                 << rDms.finalEndX << " " << rDms.finalEndY
                 << ")"
                 << endl;
            cerr << "URegion::RIF() point restricted to iv is ("
                 << rUp.p0.GetX() << " " << rUp.p0.GetY()
                 << " "
                 << rUp.p1.GetX() << " " << rUp.p1.GetY()
                 << ")"
                 << endl;
        }

        bool ip1present;
        double ip1x;
        double ip1y;
        double ip1t;
        bool ip2present;
        double ip2x;
        double ip2y;
        double ip2t;

        specialIntersectionTrapeziumSegment(
            iv.start.ToDouble(), iv.end.ToDouble(),
            rDms.initialStartX, rDms.initialStartY,
            rDms.initialEndX, rDms.initialEndY,
            rDms.finalStartX, rDms.finalStartY,
            rDms.finalEndX, rDms.finalEndY,
            rUp.p0.GetX(), rUp.p0.GetY(),
            rUp.p1.GetX(), rUp.p1.GetY(),
            ip1present, ip1x, ip1y, ip1t,
            ip2present, ip2x, ip2y, ip2t);

        if (MRA_DEBUG) {
            cerr << "URegion::RIF() ip1present="
                 << ip1present
                 << endl;
            cerr << "URegion::RIF() ip2present="
                 << ip2present
                 << endl;
        }

        if (ip1present) {
            if (MRA_DEBUG)
                cerr << "URegion::RIF() intersection" << endl;

            if (ip2present
                && !(nearlyEqual(ip1x, ip2x)
                     && nearlyEqual(ip1y, ip2y)
                     && nearlyEqual(ip1t, ip2t))) {
                if (MRA_DEBUG)
                    cerr << "URegion::RIF() in plane" << endl;

                RestrictedIntersectionFindInPlane(
                    iv, rUp,
                    ip1present, ip1x, ip1y, ip1t,
                    ip2present, ip2x, ip2y, ip2t,
                    vtsi);
            } else {
                if (MRA_DEBUG)
                    cerr << "URegion::RIF() not in plane" << endl;

                RestrictedIntersectionFindNormal(
                    iv, rUp, rDms,
                    ip1present, ip1x, ip1y, ip1t,
                    vtsi);
            }
        }
    }
}

/*
1.1.1 Function ~RestrictedIntersectionAddUPoint()~

*/
void URegion::RestrictedIntersectionAddUPoint(MPoint& res,
                                              double starttime,
                                              double endtime,
                                              bool lc,
                                              bool rc,
                                              double x0,
                                              double y0,
                                              double x1,
                                              double y1,
                                              UPoint*& pending) const {
    if (MRA_DEBUG) {
        cerr << "URegion::RIAUP() called" << endl;
        cerr << "URegion::RIAUP() starttime=" << starttime
             << " endtime=" << endtime
             << " lc=" << lc << " rc="
             << rc
             << " p0=(" << x0 << " " << y0
             << ") p1=(" << x1 << " " << y1
             << ")"
             << endl;
    }

    Instant start(instanttype);
    start.ReadFrom(starttime);

    Instant end(instanttype);
    end.ReadFrom(endtime);

/*
Straightforward again. If there is a previous unit pending, try to merge
it. If we cannot merge it, put it into ~res~ and start new pending
unit.

(I am not sure, though, whether merging pending units is required. It may
reduce the number of units produced - but is this really a requirement?)

*/
    if (pending) {
        if (MRA_DEBUG)
            cerr << "URegion::RIAUP() pending exists" << endl;

        if (nearlyEqual(pending->timeInterval.end.ToDouble(),
                        starttime)
            && (pending->timeInterval.rc || lc)
            && nearlyEqual(pending->p1.GetX(), x0)
            && nearlyEqual(pending->p1.GetY(), y0)) {

            if (MRA_DEBUG)
                cerr << "URegion::RIAUP() intervals match" << endl;

            if (nearlyEqual(pending->timeInterval.start.ToDouble(),
                            pending->timeInterval.end.ToDouble())) {
                if (MRA_DEBUG)
                    cerr << "URegion::RIAUP() merge #1" << endl;

                Interval<Instant> iv(start,
                                     end,
                                     true,
                                     rc);

                delete pending;
                pending = new UPoint(iv, x0, y0, x1, y1);

                if (MRA_DEBUG)
                    cerr << "URegion::RIAUP() pending="
                         << (unsigned int) pending
                         << endl;

                return;
            } else if (nearlyEqual(starttime, endtime)) {
                if (MRA_DEBUG)
                    cerr << "URegion::RIAUP() merge #2" << endl;

                Interval<Instant> iv(pending->timeInterval.start,
                                     pending->timeInterval.end,
                                     pending->timeInterval.lc,
                                     true);

                UPoint* dummy = new UPoint(iv,
                                           pending->p0.GetX(),
                                           pending->p0.GetY(),
                                           pending->p1.GetX(),
                                           pending->p1.GetY());
                delete pending;
                pending = dummy;

                    if (MRA_DEBUG)
                        cerr << "URegion::RIAUP() pending="
                             << (unsigned int) pending
                             << endl;

                return;
            } else {
                if (MRA_DEBUG)
                    cerr << "URegion::RIAUP() checking endpoints"
                         << endl;

                double f =
                    (pending->timeInterval.start.ToDouble()
                     -starttime)
                    /(endtime-starttime);

                double x = x0+(x1-x0)*f;
                double y = y0+(y1-y0)*f;

                if (nearlyEqual(pending->p0.GetX(), x)
                    && nearlyEqual(pending->p0.GetY(), y)) {
                    if (MRA_DEBUG)
                        cerr << "URegion::RIAUP() merge #3" << endl;

                    Interval<Instant> iv(pending->timeInterval.start,
                                         end,
                                         pending->timeInterval.lc,
                                         rc);

                    UPoint* dummy = new UPoint(iv,
                                               pending->p0.GetX(),
                                               pending->p0.GetY(),
                                               x1,
                                               y1);
                    delete pending;
                    pending = dummy;

                    if (MRA_DEBUG)
                        cerr << "URegion::RIAUP() pending="
                             << (unsigned int) pending
                             << endl;

                    return;
                }
            }
        }
    }

    if (MRA_DEBUG) cerr << "URegion::RIAUP() no merge" << endl;

    if (pending) {
        if (MRA_DEBUG)
            cerr << "URegion::RIAUP() adding "
                 << (unsigned int) pending
                 << " iv=["
                 << pending->timeInterval.start.ToDouble()
                 << " "
                 << pending->timeInterval.end.ToDouble()
                 << " "
                 << pending->timeInterval.lc
                 << " "
                 << pending->timeInterval.rc
                 << "]"
                 << endl;

        if (!(nearlyEqual(pending->timeInterval.start.ToDouble(),
                          pending->timeInterval.end.ToDouble())
              && (!pending->timeInterval.lc
                  || !pending->timeInterval.rc)))
            res.Add(*pending);

        delete pending;
    }

    Interval<Instant> iv(start, end, lc, rc);

    pending = new UPoint(iv, x0, y0, x1, y1);

    if (MRA_DEBUG)
        cerr << "URegion::RIAUP() pending="
             << (unsigned int) pending
             << endl;

}

/*
1.1.1 Function ~RestrictedIntersectionProcess()~

*/
bool URegion::RestrictedIntersectionProcess(
    const UPoint& up,
    const Interval<Instant>& iv,
    vector<TrapeziumSegmentIntersection>& vtsi,
    MPoint& res,
    UPoint*& pending) const {

/*
Check all intersections with the same coordinates. Bring them in
useful order. For example, if we are inside region at a specific time
and arrive at a $TSI\_ENTER$ and $TSI\_LEAVE$ intersection, we should
consider the $TSI\_LEAVE$ intersection first.

*/
    if (MRA_DEBUG) cerr << "URegion::RIP() called" << endl;

    if (vtsi.size() == 0) return false;

    int prev_i = -1;
    bool prev_c;

    unsigned int pos = 0;

    if (MRA_DEBUG && vtsi.size() > 0)
        cerr << "URegion::RIP() (pre) intersection dump #"
             << 0
             << " type="
             << vtsi[0].type
             << " ip=["
             << vtsi[0].x << " " << vtsi[0].y << " " << vtsi[0].t
             << "]"
             << endl;

    for (unsigned int i = 1; i < vtsi.size(); i++) {
        if (MRA_DEBUG)
            cerr << "URegion::RIP() intersection dump #"
                 << i
                 << " type="
                 << vtsi[i].type
                 << " ip=["
                 << vtsi[i].x << " " << vtsi[i].y << " " << vtsi[i].t
                 << "] pos="
                 << pos
                 << endl;

//        if (vtsi[i].type == vtsi[pos].type) {
        if (vtsi[i].type == TSI_IGNORE) {
            if (MRA_DEBUG)
                cerr << "URegion::RIP() removing" << endl;
        } else {
            if (MRA_DEBUG)
                cerr << "URegion::RIP() keeping" << endl;

            pos++;
            if (pos < i) vtsi[pos] = vtsi[i];
        }
    }

    for (unsigned int i = 0; i <= pos; i++) {
        if (MRA_DEBUG) {
            cerr << "URegion::RIP() intersection dump #"
                 << i
                 << " type="
                 << vtsi[i].type
                 << " ip=["
                 << vtsi[i].x << " " << vtsi[i].y << " " << vtsi[i].t
                 << "]"
                 << endl;
        }

        if (i == 0 && vtsi[i].type == TSI_LEAVE) {
            if (MRA_DEBUG)
                cerr << "URegion::RIP() special case (start)"
                     << endl;

            bool rc =
                nearlyEqual(vtsi[i].t, iv.end.ToDouble())
                ? iv.rc
                : true;

            UPoint rUp;
            restrictUPointToInterval(up, iv, rUp);

            RestrictedIntersectionAddUPoint(
                res,
                iv.start.ToDouble(), vtsi[i].t, iv.lc, rc,
                rUp.p0.GetX(), rUp.p0.GetY(), vtsi[i].x, vtsi[i].y,
                pending);

            prev_i = i;
            prev_c = rc;
        }

        if (i > 0 && vtsi[i].type == TSI_LEAVE) {
            bool lc =
                nearlyEqual(vtsi[i-1].t, iv.start.ToDouble())
                ? iv.lc
                : true;

            if (prev_i >= 0
                && nearlyEqual(vtsi[prev_i].t, vtsi[i-1].t)
                && prev_c)
                lc = false;

            bool rc =
                nearlyEqual(vtsi[i].t, iv.end.ToDouble())
                ? iv.rc
                : true;

            RestrictedIntersectionAddUPoint(
                res,
                vtsi[i-1].t, vtsi[i].t, lc, rc,
                vtsi[i-1].x, vtsi[i-1].y, vtsi[i].x, vtsi[i].y,
                pending);

            prev_i = i;
            prev_c = rc;
        }

        if (i == pos && vtsi[i].type == TSI_ENTER) {
            if (MRA_DEBUG)
                cerr << "URegion::RIP() special case (end)" << endl;

            bool lc =
                nearlyEqual(vtsi[i].t, iv.start.ToDouble())
                ? iv.lc
                : true;

            if (prev_i >= 0
                && nearlyEqual(vtsi[prev_i].t, vtsi[i].t)
                && prev_c)
                lc = false;

            UPoint rUp;
            restrictUPointToInterval(up, iv, rUp);

            RestrictedIntersectionAddUPoint(
                res,
                vtsi[i].t, iv.end.ToDouble(), lc, iv.rc,
                vtsi[i].x, vtsi[i].y, rUp.p1.GetX(), rUp.p1.GetY(),
                pending);

            prev_i = 0;
        }
    }

    return prev_i >= 0;
}

/*
1.1.1 Function ~RestrictedIntersectionFix()~

*/
void URegion::RestrictedIntersectionFix(
    vector<TrapeziumSegmentIntersection>& vtsi) const {

/*
Examines intersections with same coordinate, counts $TSI\_LEAVE$ and
$TSI\_ENTER$ intersections and results in one or zero intersections with
the same coordinate, based on whether there are more $TSI\_LEAVE$ or
$TSI\_ENTER$ intersections or the same number of them.

*/
    TsiType lastType;

    if (MRA_DEBUG)
        for (unsigned int i = 0; i < vtsi.size(); i++)
            cerr << "URegion::RIFix() intersection dump #"
                 << i
                 << " type="
                 << vtsi[i].type
                 << " ip=["
                 << vtsi[i].x << " " << vtsi[i].y << " " << vtsi[i].t
                 << "]"
                 << endl;


    for (unsigned int i = 0; i < vtsi.size(); i++) {
        if (MRA_DEBUG)
            cerr << "URegion::RIFix() post-sort i=" << i << endl;

        unsigned int j;
        unsigned int numEnter = 0;
        unsigned int numLeave = 0;

        double t = vtsi[i].t;
        double x = vtsi[i].x;
        double y = vtsi[i].y;

        for (j = i;
             j < vtsi.size()
                 && nearlyEqual(vtsi[j].t, t)
                 && nearlyEqual(vtsi[j].x, x)
                 && nearlyEqual(vtsi[j].y, y);
             j++) {
            if (vtsi[j].type == TSI_ENTER)
                numEnter++;
            else
                numLeave++;
        }

        j--;

        if (MRA_DEBUG) {
            cerr << "URegion::RIFix() post-sort j=" << j << endl;
            cerr << "URegion::RIFix() post-sort numEnter="
                 << numEnter
                 << endl;
            cerr << "URegion::RIFix() post-sort numLeave="
                 << numLeave
                 << endl;
        }

/*
blubb

*/
        if (j > i) {
            if (numEnter == numLeave) {
                if (i > 0 && lastType == TSI_ENTER) {
                    vtsi[i].type = TSI_LEAVE;
                    vtsi[i+1].type = TSI_ENTER;
                    lastType = TSI_ENTER;
                } else {
                    vtsi[i].type = TSI_ENTER;
                    vtsi[i+1].type = TSI_LEAVE;
                    lastType = TSI_LEAVE;
                }
                for (unsigned int k = i+2; k <= j; k++)
                    vtsi[k].type = TSI_IGNORE;
            } else if (numEnter > numLeave) {
                vtsi[i].type = TSI_ENTER;
                for (unsigned int k = i+1; k <= j; k++)
                    vtsi[k].type = TSI_IGNORE;
                lastType = TSI_ENTER;
            } else {
                vtsi[i].type = TSI_LEAVE;
                for (unsigned int k = i+1; k <= j; k++)
                    vtsi[k].type = TSI_IGNORE;
                lastType = TSI_LEAVE;
            }

            i = j;
        } else {
            lastType = vtsi[i].type;
        }
    }
}

/*
1.1.1.1 Method ~RestrictedIntersection()~

Checks whether the point unit ~up~ intersects this region unit, while
both units are restrictured to the interval ~iv~, which must be inside
the interval of the two units (this is not checked and must be assured
before this method is called!).

*/
void URegion::RestrictedIntersection(const UPoint& up,
                                     const Interval<Instant>& iv,
                                     MPoint& res,
                                     UPoint*& pending) const {
    if (MRA_DEBUG)
        cerr << "URegion::RI() called" << endl;

    vector<TrapeziumSegmentIntersection> vtsi;

    RestrictedIntersectionFind(up, iv, vtsi);

    if (MRA_DEBUG)
        cerr << "URegion::RI() vtsi.size()=" << vtsi.size()
             << endl;

    sort(vtsi.begin(), vtsi.end());

    RestrictedIntersectionFix(vtsi);

    if (!RestrictedIntersectionProcess(up, iv, vtsi, res, pending)) {
        if (MRA_DEBUG)
            cerr << "URegion::RI() no intersection in whole unit"
                 << endl;

        unsigned int num = Plumbline(up, iv);

        if (num > 0 && num % 2 == 1) {
            UPoint rUp;
            restrictUPointToInterval(up, iv, rUp);

            RestrictedIntersectionAddUPoint(
                res,
                iv.start.ToDouble(), iv.end.ToDouble(),
                iv.lc, iv.rc,
                rUp.p0.GetX(), rUp.p0.GetY(),
                rUp.p1.GetX(), rUp.p1.GetY(),
                pending);
        }
    }
}

/*
1.1.1 Method ~TemporalFunction()~

*/
void URegion::TemporalFunction(const Instant& t, CRegion& res) const {
    if (MRA_DEBUG)
        cerr << "URegion::TemporalFunction() called" << endl;

/*
Straightforward again. Calculate segments at specified instant,
remove degenerated segments of initial or final instant, when they
are not border of any region, and create region.

*/
    assert(t.IsDefined());
    assert(timeInterval.Contains(t));

    Instant t0 = timeInterval.start;
    Instant t1 = timeInterval.end;

    bool initialInstant = nearlyEqual(t0.ToDouble(), t.ToDouble());
    bool finalInstant = nearlyEqual(t1.ToDouble(), t.ToDouble());

    double f =
        nearlyEqual(t0.ToDouble(), t1.ToDouble())
        ? 0
        : (t-t0)/(t1-t0);

    int partnerno = 0;

    CRegion* cr = new CRegion(0);

    cr->StartBulkLoad();

    for (unsigned int i = 0; i < segmentsNum; i++) {
            if (MRA_DEBUG)
                cerr << "URegion::TemporalFunction() segment #"
                     << i
                     << " ("
                     << segmentsStartPos
                     << ")"
                     << endl;

            const MSegmentData *dms;
            segments->Get(segmentsStartPos+i, dms);

            double xs =
                dms->initialStartX
                +(dms->finalStartX-dms->initialStartX)*f;
            double ys =
                dms->initialStartY
                +(dms->finalStartY-dms->initialStartY)*f;
            double xe =
                dms->initialEndX
                +(dms->finalEndX-dms->initialEndX)*f;
            double ye =
                dms->initialEndY
                +(dms->finalEndY-dms->initialEndY)*f;

            if (MRA_DEBUG)
                cerr << "URegion::TemporalFunction()   value is "
                     << xs << " " << ys << " " << xe << " " << ye
                     << endl;

            if (nearlyEqual(xs, xe) && nearlyEqual(ys, ye)) {
                if (MRA_DEBUG)
                    cerr << "URegion::TemporalFunction()   "
                         << "reduced to point"
                         << endl;
                continue;
            }

            assert(dms->degeneratedInitial != DGM_UNKNOWN);
            assert(dms->degeneratedFinal != DGM_UNKNOWN);

            if ((initialInstant
                 && dms->degeneratedInitial == DGM_IGNORE)
                || (finalInstant
                    && dms->degeneratedFinal == DGM_IGNORE)) {
                if (MRA_DEBUG)
                    cerr << "URegion::TemporalFunction()   "
                         << "ignored degenerated"
                         << endl;
                continue;
            }

            Point s(true, xs, ys);
            Point e(true, xe, ye);
            CHalfSegment chs(true, true, s, e);

            chs.attr.partnerno = partnerno++;

            if (initialInstant
                 && dms->degeneratedInitial == DGM_INSIDEABOVE)
                chs.attr.insideAbove = true;
            else if (initialInstant
                 && dms->degeneratedInitial == DGM_NOTINSIDEABOVE)
                chs.attr.insideAbove = false;
            else if (finalInstant
                 && dms->degeneratedFinal == DGM_INSIDEABOVE)
                chs.attr.insideAbove = true;
            else if (finalInstant
                 && dms->degeneratedFinal == DGM_NOTINSIDEABOVE)
                chs.attr.insideAbove = false;
            else
                chs.attr.insideAbove = dms->insideAbove;

            *cr += chs;

            chs.SetLDP(false);

            *cr += chs;
    }

    cr->EndBulkLoad();

//    cr->Sort();
    cr->SetPartnerNo();

    if (MRA_DEBUG)
        for (int i = 0; i < cr->Size(); i++) {
            const CHalfSegment *chs;
            cr->Get(i, chs);

            cerr << "URegion::TemporalFunction() segment #"
                 << i
                 << " lp=("
                 << chs->GetLP().GetX()
                 << ", "
                 << chs->GetLP().GetY()
                 << ") rp=("
                 << chs->GetRP().GetX()
                 << ", "
                 << chs->GetRP().GetY()
                 << ") ldp="
                 << chs->GetLDP()
                 << " attr="
                 << chs->attr.faceno
                 << " "
                 << chs->attr.cycleno
                 << " "
                 << chs->attr.edgeno
                 << " "
                 << chs->attr.partnerno
                 << " "
                 << chs->attr.insideAbove
                 << endl;
        }

    cr->ComputeRegion();

    if (MRA_DEBUG)
        for (int i = 0; i < cr->Size(); i++) {
            const CHalfSegment *chs;
            cr->Get(i, chs);

            cerr << "URegion::TemporalFunction() segment #"
                 << i
                 << " lp=("
                 << chs->GetLP().GetX()
                 << ", "
                 << chs->GetLP().GetY()
                 << ") rp=("
                 << chs->GetRP().GetX()
                 << ", "
                 << chs->GetRP().GetY()
                 << ") ldp="
                 << chs->GetLDP()
                 << " attr="
                 << chs->attr.faceno
                 << " "
                 << chs->attr.cycleno
                 << " "
                 << chs->attr.edgeno
                 << " "
                 << chs->attr.partnerno
                 << " "
                 << chs->attr.insideAbove
                 << endl;
        }

/*
This is quite ugly and may not work with other compilers than gcc.
Since the ~Intime<Alpha>()~ constructors do not properly initial their
~value~ attribute (which if of type ~CRegion~ in this case), there is
no better solution right now.

*/
    memcpy(&res, cr, sizeof(*cr));
    free(cr);
}

/*
1.1.1 Method ~AddSegment()~

*/
bool URegion::AddSegment(CRegion& cr,
                         CRegion& rDir,
                         unsigned int faceno,
                         unsigned int cycleno,
                         unsigned int segmentno,
                         unsigned int partnerno,
                         double intervalLen,
                         ListExpr start,
                         ListExpr end) {
    if (MRA_DEBUG)
        cerr << "URegion::AddSegment() called "
             << faceno
             << " "
             << cycleno
             << " "
             << segmentno
             << endl;

    assert(role == NORMAL || role == EMBEDDED);

/*
To avoid awkward return value handling, we throw an exception if we find
a mistake. This is caught and converted into a proper error message and
return value.

*/

    try {
/*
Check list representation.

*/
        if (nl->ListLength(start) != 4
            || !nl->IsAtom(nl->First(start))
            || (nl->AtomType(nl->First(start)) != RealType
                && nl->AtomType(nl->First(start)) != IntType)
            || !nl->IsAtom(nl->Second(start))
            || (nl->AtomType(nl->Second(start)) != RealType
                && nl->AtomType(nl->Second(start)) != IntType)
            || !nl->IsAtom(nl->Third(start))
            || (nl->AtomType(nl->Third(start)) != RealType
                && nl->AtomType(nl->Third(start)) != IntType)
            || !nl->IsAtom(nl->Fourth(start))
            || (nl->AtomType(nl->Fourth(start)) != RealType
                && nl->AtomType(nl->Fourth(start)) != IntType))
            throw invalid_argument(
                "start point "
                +nl->ToString(start)
                +" not in format (<number> <number> <number> <number>)");

        if (nl->ListLength(end) != 4

            || !nl->IsAtom(nl->First(end))
            || (nl->AtomType(nl->First(end)) != RealType
                && nl->AtomType(nl->First(end)) != IntType)
            || !nl->IsAtom(nl->Second(end))
            || (nl->AtomType(nl->Second(end)) != RealType
                && nl->AtomType(nl->Second(end)) != IntType)
            || !nl->IsAtom(nl->Third(end))
            || (nl->AtomType(nl->Third(end)) != RealType
                && nl->AtomType(nl->Third(end)) != IntType)
            || !nl->IsAtom(nl->Fourth(end))
            || (nl->AtomType(nl->Fourth(end)) != RealType
                && nl->AtomType(nl->Fourth(end)) != IntType))
            throw invalid_argument(
                "end point "
                +nl->ToString(end)
                +" not in format (<number> <number> <number> <number>)");

/*
Create segment from list representation.

*/
        MSegmentData dms(faceno,
                         cycleno,
                         segmentno,
                         false,
                         nl->AtomType(nl->First(start)) == RealType
                           ? nl->RealValue(nl->First(start))
                           : (double) nl->IntValue(nl->First(start)),
                         nl->AtomType(nl->Second(start)) == RealType
                           ? nl->RealValue(nl->Second(start))
                           : (double) nl->IntValue(nl->Second(start)),
                         nl->AtomType(nl->First(end)) == RealType
                           ? nl->RealValue(nl->First(end))
                           : (double) nl->IntValue(nl->First(end)),
                         nl->AtomType(nl->Second(end)) == RealType
                           ? nl->RealValue(nl->Second(end))
                           : (double) nl->IntValue(nl->Second(end)),
                         nl->AtomType(nl->Third(start)) == RealType
                           ? nl->RealValue(nl->Third(start))
                           : (double) nl->IntValue(nl->Third(start)),
                         nl->AtomType(nl->Fourth(start)) == RealType
                           ? nl->RealValue(nl->Fourth(start))
                           : (double) nl->IntValue(nl->Fourth(start)),
                         nl->AtomType(nl->Third(end)) == RealType
                           ? nl->RealValue(nl->Third(end))
                           : (double) nl->IntValue(nl->Third(end)),
                         nl->AtomType(nl->Fourth(end)) == RealType
                           ? nl->RealValue(nl->Fourth(end))
                           : (double) nl->IntValue(nl->Fourth(end)));

        if (MRA_DEBUG)
            cerr << "URegion::AddSegment() segmentsNum="
                 << segmentsNum
                 << endl;

/*
For each of the already existing segments:

*/
        for (int i = segmentsNum-1; i >= 0; i--) {
            if (MRA_DEBUG)
                cerr << "URegion::AddSegment() i=" << i << endl;

            const MSegmentData *auxExistingDms;

            segments->Get(segmentsStartPos+i, auxExistingDms);
            MSegmentData existingDms( *auxExistingDms );

/*
Check whether the current segment degenerates with this segment in the
initial instant. Note that segments reduced to points are excluded from
this.

All segments, which degenerate into each other, are collected in a list
using the ~degeneratedInitialNext~ attribute.

*/
            if (dms.degeneratedInitialNext < 0
                && !dms.pointInitial
                && !existingDms.pointInitial
                && ((nearlyEqual(
                         dms.initialStartX,
                         existingDms.initialStartX)
                     && nearlyEqual(
                            dms.initialStartY,
                            existingDms.initialStartY)
                     && nearlyEqual(
                            dms.initialEndX,
                            existingDms.initialEndX)
                     && nearlyEqual(
                            dms.initialEndY,
                            existingDms.initialEndY))
                    || (nearlyEqual(
                            dms.initialStartX,
                            existingDms.initialEndX)
                        && nearlyEqual(
                                dms.initialStartY,
                                existingDms.initialEndY)
                        && nearlyEqual(
                                dms.initialEndX,
                                existingDms.initialStartX)
                        && nearlyEqual(
                                dms.initialEndY,
                                existingDms.initialStartY)))) {
                if (MRA_DEBUG)
                    cerr << "URegion::AddSegment() "
                         << "degen'ed initial in " << i
                         << endl;

                dms.degeneratedInitialNext = 0;
                existingDms.degeneratedInitialNext = segmentsNum+1;

                segments->Put(segmentsStartPos+i, existingDms);
            }

/*
Same for the final instant.

*/
            if (dms.degeneratedFinalNext < 0
                && !dms.pointFinal
                && !existingDms.pointFinal
                && ((nearlyEqual(
                         dms.finalStartX,
                         existingDms.finalStartX)
                     && nearlyEqual(
                            dms.finalStartY,
                            existingDms.finalStartY)
                     && nearlyEqual(
                            dms.finalEndX,
                            existingDms.finalEndX)
                     && nearlyEqual(
                            dms.finalEndY,
                            existingDms.finalEndY))
                    || (nearlyEqual(
                            dms.finalStartX,
                            existingDms.finalEndX)
                        && nearlyEqual(
                               dms.finalStartY,
                               existingDms.finalEndY)
                        && nearlyEqual(
                               dms.finalEndX,
                               existingDms.finalStartX)
                        && nearlyEqual(
                               dms.finalEndY,
                               existingDms.finalStartY)))) {
                if (MRA_DEBUG)
                    cerr << "URegion::AddSegment() "
                         << "degen'ed final in " << i
                         << endl;

                dms.degeneratedFinalNext = 0;
                existingDms.degeneratedFinalNext = segmentsNum+1;

                segments->Put(segmentsStartPos+i, existingDms);
            }

/*
If we have a point time interval, degeneration is not allowed.

*/
            if (nearlyEqual(intervalLen, 0.0)
                && (dms.degeneratedInitialNext >= 0
                    || dms.degeneratedFinalNext >= 0))
                throw invalid_argument(
                    "units with zero length time interval must not "
                    "be degenerated");
            else if (!nearlyEqual(intervalLen, 0.0)
                     && dms.degeneratedInitialNext >= 0
                     && dms.degeneratedFinalNext >= 0)
                throw invalid_argument(
                    "units must not degenerate both in initial and "
                    "final instant");

/*
Check if the current segment intersects with the existing segment.
Since both a moving segments and are spanning a trapezium in 3d space
$(x, y, t)$, this is equivalent to the intersection of two trapeziums.

*/
            unsigned int detailedResult;

            if (specialTrapeziumIntersects(intervalLen,
                                           existingDms.initialStartX,
                                           existingDms.initialStartY,
                                           existingDms.initialEndX,
                                           existingDms.initialEndY,
                                           existingDms.finalStartX,
                                           existingDms.finalStartY,
                                           existingDms.finalEndX,
                                           existingDms.finalEndY,
                                           dms.initialStartX,
                                           dms.initialStartY,
                                           dms.initialEndX,
                                           dms.initialEndY,
                                           dms.finalStartX,
                                           dms.finalStartY,
                                           dms.finalEndX,
                                           dms.finalEndY,
                                           detailedResult)) {
                cerr << "existing segment: ("
                     << existingDms.initialStartX
                     << " " << existingDms.initialStartY
                     << " " << existingDms.initialEndX
                     << " " << existingDms.initialEndY
                     << ")-("
                     << existingDms.initialStartX
                     << " " << existingDms.initialStartY
                     << " " << existingDms.initialEndX
                     << " " << existingDms.initialEndY
                     << ")"
                     << endl;

                stringstream msg;
                msg << "moving segments intersect (code "
                    << detailedResult
                    << ")";
                throw invalid_argument(msg.str());
            }
        }

/*
Add half segments to $cr$ and $rDir$ for region check and direction
computation.

*/
        double t = nearlyEqual(intervalLen, 0.0) ? 0 : 0.5;
        double xs =
            dms.initialStartX+(dms.finalStartX-dms.initialStartX)*t;
        double ys =
            dms.initialStartY+(dms.finalStartY-dms.initialStartY)*t;
        double xe =
            dms.initialEndX+(dms.finalEndX-dms.initialEndX)*t;
        double ye =
            dms.initialEndY+(dms.finalEndY-dms.initialEndY)*t;

        assert(!nearlyEqual(xs, xe) || !nearlyEqual(ys, ye));

        Point s(true, xs, ys);
        Point e(true, xe, ye);
        CHalfSegment chs(true, true, s, e);

        chs.attr.faceno = faceno;
        chs.attr.cycleno = cycleno;
        chs.attr.partnerno = partnerno;

        chs.attr.insideAbove = chs.GetLP() == s;

        if (MRA_DEBUG) {
            cerr << "URegion::AddSegment() initial "
                 << dms.initialStartX
                 << " "
                 << dms.initialStartY
                 << " "
                 << dms.initialEndX
                 << " "
                 << dms.initialEndY
                 << endl;
            cerr << "URegion::AddSegment() final "
                 << dms.finalStartX
                 << " "
                 << dms.finalStartY
                 << " "
                 << dms.finalEndX
                 << " "
                 << dms.finalEndY
                 << endl;
        }

        if (MRA_DEBUG)
            cerr << "URegion::AddSegment() "
                 << faceno
                 << " "
                 << cycleno
                 << " "
                 << partnerno
                 << " ("
                 << xs
                 << ", "
                 << ys
                 << ")-("
                 << xe
                 << ", "
                 << ye
                 << ") ia="
                 << chs.attr.insideAbove
                 << endl;

        if (!cr.insertOK(chs))
            throw
                invalid_argument(
                    "CRegion checks for segment failed");

        cr += chs;
        rDir += chs;

        chs.SetLDP(false);
        cr += chs;

        segments->Resize(segmentsStartPos+segmentsNum+1);
        segments->Put(segmentsStartPos+segmentsNum, dms);
        segmentsNum++;
    } catch (invalid_argument& e) {
        cerr << "checking segment "
             << nl->ToString(start)
             << " - "
             << nl->ToString(end)
             << " failed: "
             << e.what()
             << endl;
        return false;
    }

    return true;
}

/*

1.1.1 Attribute access methods

1.1.1.1 Method ~SetSegmentInsideAbove()~

*/
void URegion::SetSegmentInsideAbove(int pos, bool insideAbove) {
    if (MRA_DEBUG)
        cerr << "URegion::GetSegmentLeftOrAbove() called" << endl;

    const MSegmentData *auxDms;
    segments->Get(segmentsStartPos+pos, auxDms);
    MSegmentData dms( *auxDms );
    dms.insideAbove = insideAbove;
    segments->Put(segmentsStartPos+pos, dms);
}

/*
1.1.1.1 Method ~GetSegmentsNum()~

*/
int URegion::GetSegmentsNum(void) const {
    if (MRA_DEBUG)
        cerr << "URegion::GetSegmentsNum() called, num="
             << segmentsNum
             << endl;

    assert(role == NORMAL || role == EMBEDDED);

    return segmentsNum;
}

/*
1.1.1.1 Method ~GetSegment()~

*/
void URegion::GetSegment(int pos, const MSegmentData*& dms) const {
    if (MRA_DEBUG)
        cerr << "URegion::GetSegment() called, pos=" << pos
             << endl;

    assert(role == NORMAL || role == EMBEDDED);

    segments->Get(segmentsStartPos+pos, dms);
}

/*
1.1.1.1 Method ~PutSegment()~

*/
void URegion::PutSegment(int pos, const MSegmentData& dms) {
    if (MRA_DEBUG)
        cerr << "URegion::PutSegment() called, pos=" << pos
             << endl;

    assert(role == NORMAL || role == EMBEDDED);

    segments->Put(segmentsStartPos+pos, dms);
}

/*
1.1 Algebra integration

1.1.1 Function ~URegionProperty()~

*/

// *hm* Verify list representation

static ListExpr URegionProperty() {
    if (MRA_DEBUG) cerr << "URegionProperty() called" << endl;

    ListExpr listrep = nl->TextAtom();
    nl->AppendText(listrep,
                   "(<interval> <face>*), where <interval> is "
                   "(<real> <real> <bool> <bool>) and where "
                   "<face> is (<outercycle> <holecycle>*), "
                   "where <outercycle> and <holecycle> are "
                   "(<real> <real> <real> <real>), representing "
                   "start X, start Y, end X and end Y values.");
    ListExpr example = nl->TextAtom();
    nl->AppendText(example,
                   "((0.0 10.0 TRUE TRUE)"
                   "((((1.0 3.5 1.5 1.5)"
                   "(2.0 5.5 3.0 4.5)"
                   "(3.0 6.5 3.5 5.0)"
                   "(4.0 6.5 5.5 5.0)"
                   "(4.0 5.5 5.5 4.5)"
                   "(5.0 4.5 7.5 2.5)"
                   "(5.0 2.5 7.5 1.0)"
                   "(4.0 1.5 7.0 0.5)"
                   "(3.0 1.5 2.5 0.5))"
                   "((2.0 3.0 3.0 2.0)"
                   "(2.0 4.0 3.0 3.0)"
                   "(3.0 4.0 4.0 3.0)"
                   "(3.0 3.0 4.0 2.0)))))");
    ListExpr remarks = nl->TextAtom();
    nl->AppendText(remarks,
                   "All <holecycle> must be completely within "
                   "corresponding <outercylce>.");

    return
        nl->TwoElemList(
            nl->FiveElemList(
                nl->StringAtom("Signature"),
                nl->StringAtom("Example Type List"),
                nl->StringAtom("List Rep"),
                nl->StringAtom("Example List"),
                nl->StringAtom("Remarks")),
            nl->FiveElemList(
                nl->StringAtom("-> UNIT"),
                nl->StringAtom("(uregion)"),
                listrep,
                example,
                remarks));
}

/*
1.1.1 Function ~CheckURegion()~

*/
static bool CheckURegion(ListExpr type, ListExpr& errorInfo) {
    if (MRA_DEBUG) cerr << "CheckURegion() called" << endl;

    return nl->IsEqual(type, "uregion");
}

/*
1.1.1 Function ~OutURegion()~

*/
static ListExpr OutURegion(ListExpr typeInfo, Word value) {
     if (MRA_DEBUG) cerr << "OutURegion() called" << endl;

/*
Conversio to list representation is straightforward. Just loop through
faces, cylces and segments and make sure that it is realised when one of
these changes.

*/
    URegion* ur = (URegion*) value.addr;

    int num = ur->GetSegmentsNum();

    ListExpr faces = nl->TheEmptyList();
    ListExpr facesLastElem = faces;

    ListExpr face = nl->TheEmptyList();
    ListExpr faceLastElem = face;
    ListExpr cycle = nl->TheEmptyList();
    ListExpr cycleLastElem = cycle;

    for (int i = 0; i < num; i++) {
        if (MRA_DEBUG)
            cerr << "OutURegion() segment #" << i << endl;

        const MSegmentData *dms;
        ur->GetSegment(i, dms);

        if (MRA_DEBUG)
            cerr << "OutURegion() point is "
                 << dms->GetFaceNo()
                 << " "
                 << dms->GetCycleNo()
                 << " ("
                 << dms->GetInitialStartX()
                 << ", "
                 << dms->GetInitialStartY()
                 << ", "
                 << dms->GetFinalStartX()
                 << ", "
                 << dms->GetFinalStartY()
                 << ")"
                 << endl;

        ListExpr p =
            nl->FourElemList(
                nl->RealAtom(dms->GetInitialStartX()),
                nl->RealAtom(dms->GetInitialStartY()),
                nl->RealAtom(dms->GetFinalStartX()),
                nl->RealAtom(dms->GetFinalStartY()));

        if (cycle == nl->TheEmptyList()) {
            if (MRA_DEBUG) cerr << "OutURegion() new cycle" << endl;
            cycle = nl->OneElemList(p);
            cycleLastElem = cycle;
        } else {
            if (MRA_DEBUG)
                cerr << "OutURegion() existing cycle" << endl;
            cycleLastElem = nl->Append(cycleLastElem, p);
        }

        const MSegmentData *nextDms;
        if (i < num-1) ur->GetSegment(i+1, nextDms);

        if (i == num-1
            || dms->GetCycleNo() != nextDms->GetCycleNo()
            || dms->GetFaceNo() != nextDms->GetFaceNo()) {
            if (MRA_DEBUG)
                cerr << "OutURegion() end of cycle" << endl;

            if (face == nl->TheEmptyList()) {
                if (MRA_DEBUG)
                    cerr << "OutURegion() new face" << endl;
                face = nl->OneElemList(cycle);
                faceLastElem = face;
            } else {
                if (MRA_DEBUG)
                    cerr << "OutURegion() existing face" << endl;
                faceLastElem = nl->Append(faceLastElem, cycle);
            }

            if (i == num-1
                || dms->GetFaceNo() != nextDms->GetFaceNo()) {
                if (MRA_DEBUG)
                    cerr << "OutURegion() end of face" << endl;
                if (faces == nl->TheEmptyList()) {
                    faces = nl->OneElemList(face);
                    facesLastElem = faces;
                } else
                    facesLastElem = nl->Append(facesLastElem, face);

                face = nl->TheEmptyList();
                faceLastElem = face;
            }

            cycle = nl->TheEmptyList();
            cycleLastElem = cycle;
        }
    }

    ListExpr res =
        nl->TwoElemList(
            nl->FourElemList(
                OutDateTime(nl->TheEmptyList(),
                            SetWord(&ur->timeInterval.start)),
                OutDateTime(nl->TheEmptyList(),
                            SetWord(&ur->timeInterval.end)),
                nl->BoolAtom(ur->timeInterval.lc),
                nl->BoolAtom(ur->timeInterval.rc)),
            faces);

    return res;
}

/*
1.1.1 Function ~InURegionEmbedded()~

*/
static Word InURegionEmbedded(const ListExpr typeInfo,
                              const ListExpr instance,
                              const int errorPos,
                              ListExpr& errorInfo,
                              bool& correct,
                              DBArray<MSegmentData>* segments,
                              unsigned int segmentsStartPos) {
    if (MRA_DEBUG)
        cerr << "InURegionEmbedded() called, segmentsStartPos="
             << segmentsStartPos
             << endl;

/*
Please that ~CRegion~ creation is done as shown in ~SpatialAlgebra~.
See there for more details.

*/
    if (nl->ListLength(instance) == 0) {
        cerr << "uregion not in format (<interval> <face>*)" << endl;
        correct = false;
        return SetWord(Address(0));
    }

    if (MRA_DEBUG)
        cerr << "InURegion() (<interval> <face>*) found" << endl;

    ListExpr interval = nl->First(instance);

    if (nl->ListLength(interval) != 4
        || !nl->IsAtom(nl->First(interval))
        || (nl->AtomType(nl->First(interval)) != StringType
            && nl->AtomType(nl->First(interval)) != RealType)
        || !nl->IsAtom(nl->Second(interval))
        || (nl->AtomType(nl->Second(interval)) != StringType
            && nl->AtomType(nl->Second(interval)) != RealType)
        || !nl->IsAtom(nl->Third(interval))
        || nl->AtomType(nl->Third(interval)) != BoolType
        || !nl->IsAtom(nl->Fourth(interval))
        || nl->AtomType(nl->Fourth(interval)) != BoolType) {
        cerr << "uregion interval not in format "
             << "(<real> <real> <bool> <bool>)"
             << endl;
        correct = false;
        return SetWord(Address(0));
    }

    Instant *start =
        (Instant *) InInstant(nl->TheEmptyList(),
                              nl->First(interval),
                              errorPos,
                              errorInfo,
                              correct).addr;
    if (!correct) {
        cerr << "uregion interval invalid start time" << endl;
        correct = false;
        return SetWord(Address(0));
    }

    Instant *end =
        (Instant *) InInstant(nl->TheEmptyList(),
                              nl->Second(interval),
                              errorPos,
                              errorInfo,
                              correct ).addr;
    if (!correct) {
        cerr << "uregion interval invalid end time" << endl;
        correct = false;
        delete start;
        return SetWord(Address(0));
    }

    bool lc = nl->BoolValue(nl->Third(interval));
    bool rc = nl->BoolValue(nl->Fourth(interval));

    Interval<Instant> tinterval(*start, *end, lc, rc);

    double intervalLen = end->ToDouble()-start->ToDouble();

    delete start;
    delete end;

/*
Create ~URegion~ instance and pass storage of segments, if we received
any.

*/
    URegion* uregion =
        segments == 0
        ? new URegion(tinterval)
        : new URegion(tinterval, segments, segmentsStartPos);

    unsigned int faceno = 0;
    unsigned int partnerno = 0;
    ListExpr faces = nl->Second(instance);

    CRegion cr(0);
    cr.StartBulkLoad();

    if (nl->ListLength(faces) == 0) {
        cerr << "uregion should contain at least one face" << endl;
        delete uregion;
        correct = false;
        return SetWord(Address(0));
    }

/*
... all this is similar to ~CRegion~ creation in ~SpatialAlgebra~...

*/
    while (!nl->IsEmpty(faces)) {
        if (MRA_DEBUG)
            cerr << "InURegion() face #" << faceno << endl;

        ListExpr cycles = nl->First(faces);

        if (nl->ListLength(cycles) == 0) {
            cerr << "uregion face should contain at least one cycle"
                 << endl;
            delete uregion;
            correct = false;
            return SetWord(Address(0));
        }

        unsigned int cycleno = 0;
        unsigned int pointno;

        while (!nl->IsEmpty(cycles)) {
            if (MRA_DEBUG)
                cerr << "InURegion()   cycle #" << cycleno << endl;

            pointno = 0;

            CRegion rDir(0);
            rDir.StartBulkLoad();

            ListExpr cyclepoints = nl->First(cycles);

            if (nl->ListLength(cyclepoints) < 3) {
                cerr << "uregion cycle should contain at "
                     << "least three points"
                     << endl;
                delete uregion;
                correct = false;
                return SetWord(Address(0));
            }

            ListExpr firstPoint = nl->First(cyclepoints);
            ListExpr prevPoint = 0;

            unsigned int initialSegmentsNum =
                uregion->GetSegmentsNum();

            while (!nl->IsEmpty(cyclepoints)) {
                if (MRA_DEBUG)
                    cerr << "InURegion()     point #"
                         << pointno
                         << endl;

                ListExpr point = nl->First(cyclepoints);

                if (prevPoint != 0
                    && !uregion->AddSegment(cr,
                                            rDir,
                                            faceno,
                                            cycleno,
                                            pointno,
                                            partnerno,
                                            intervalLen,
                                            prevPoint,
                                            point)) {
                    cerr << "uregion's segment checks failed"
                         << endl;
                    delete uregion;
                    correct = false;
                    return SetWord(Address(0));
                }

                prevPoint = point;
                cyclepoints = nl->Rest(cyclepoints);
                pointno++;
                partnerno++;
            }

            if (!uregion->AddSegment(cr,
                                     rDir,
                                     faceno,
                                     cycleno,
                                     pointno,
                                     partnerno,
                                     intervalLen,
                                     prevPoint,
                                     firstPoint)) {
                cerr << "uregion's segment checks failed"
                     << endl;
                delete uregion;
                correct = false;
                return SetWord(Address(0));
            }

            partnerno++;

            rDir.EndBulkLoad();

            bool direction = rDir.GetCycleDirection();

            int h = cr.Size()-(rDir.Size()*2);
            int i = initialSegmentsNum;
            while (h < cr.Size()) {
                const CHalfSegment *chsInsideAbove;
                bool insideAbove;

                cr.Get(h, chsInsideAbove);

                if (MRA_DEBUG)
                    cerr << "InURegion() i="
                         << i
                         << " insideAbove="
                         << chsInsideAbove->attr.insideAbove
                         << endl;

                if (direction == chsInsideAbove->attr.insideAbove)
                    insideAbove = false;
                else
                    insideAbove = true;
                if (cycleno > 0) insideAbove = !insideAbove;

                //chsInsideAbove.attr.insideAbove = insideAbove;
                //cr.UpdateAttr(h, chsInsideAbove.attr);

                //cr.Get(h+1, chsInsideAbove);
                //chsInsideAbove.attr.insideAbove = insideAbove;
                //cr.UpdateAttr(h+1, chsInsideAbove.attr);

                uregion->SetSegmentInsideAbove(i, insideAbove);

                h += 2;
                i++;
            }

            cycles = nl->Rest(cycles);
            cycleno++;
        }

        faces = nl->Rest(faces);
        faceno++;
    }

    cr.EndBulkLoad();

    bool nonTrivialInitial = false;
    bool nonTrivialFinal = false;

    if (MRA_DEBUG)
        for (int i = 0; i < uregion->GetSegmentsNum(); i++) {
            const MSegmentData *dms;
            uregion->GetSegment(i, dms);

            cerr << "InURegion() segment #"
                 << i
                 << ": " << dms->faceno
                 << " " << dms->cycleno
                 << " " << dms->segmentno
                 << " i=("
                 << dms->initialStartX << ", " << dms->initialStartY
                 << ", "
                 << dms->initialEndX << ", " << dms->initialEndY
                 << ") f=("
                 << dms->finalStartX << ", " << dms->finalStartY
                 << ", "
                 << dms->finalEndX << ", " << dms->finalEndY
                 << ") flags="
                 << dms->insideAbove
                 << " " << dms->pointInitial
                 << " " << dms->pointFinal
                 << " " << dms->degeneratedInitialNext
                 << " " << dms->degeneratedFinalNext
                 << endl;
        }

/*
This is different from ~CRegion~ handling in ~SpatialAlgebra~. We have
to go through the lists of degenerated segments and count how often
a region is inside above in each list. If there are more inside above
segments than others, we need one inside above segment for the
~TemporalFunction()~ and set all others to ignore. Vice version if there
are more inside below segments. If there is the same number of inside above
and inside below segments, we can ignore the entire list.

*/
    for (int i = 0; i < uregion->GetSegmentsNum(); i++) {
        const MSegmentData *auxDms;
        uregion->GetSegment(i, auxDms);
        MSegmentData dms( *auxDms );

        if (!dms.pointInitial && dms.degeneratedInitialNext < 0)
            nonTrivialInitial = true;
        if (!dms.pointFinal && dms.degeneratedFinalNext < 0)
            nonTrivialFinal = true;

        if (dms.degeneratedInitial == DGM_UNKNOWN) {
            if (dms.degeneratedInitialNext >= 0) {
                const MSegmentData *auxDegenDms;
                MSegmentData degenDms;
                unsigned int numInsideAbove = 0;
                unsigned int numNotInsideAbove = 0;
                for (int j = i+1;
                     j != 0;
                     j = degenDms.degeneratedInitialNext) {
                    uregion->GetSegment(j-1, auxDegenDms);
                    degenDms = *auxDegenDms;
                    if (MRA_DEBUG)
                        cerr << "InURegion() degen-magic-i "
                             << i
                             << " "
                             << j-1
                             << endl;
                    if (degenDms.insideAbove)
                        numInsideAbove++;
                    else
                        numNotInsideAbove++;
                    if (j != i+1) {
                        degenDms.degeneratedInitial = DGM_IGNORE;
                        uregion->PutSegment(j-1, degenDms);
                    }
                }

                if (MRA_DEBUG)
                    cerr << "InURegion() degen-magic-i result "
                         << numInsideAbove
                         << " "
                         << numNotInsideAbove
                         << endl;

                if (numInsideAbove == numNotInsideAbove) {
                    dms.degeneratedInitial = DGM_IGNORE;
                } else if (numInsideAbove == numNotInsideAbove+1) {
                    dms.degeneratedInitial = DGM_INSIDEABOVE;
                } else if (numInsideAbove+1 == numNotInsideAbove) {
                    dms.degeneratedInitial = DGM_NOTINSIDEABOVE;
                } else {
                    cerr << "segment ("
                         << dms.initialStartX
                         << ", " << dms.initialStartY
                         << ")-(" << dms.initialEndX
                         << ", " << dms.initialEndY
                         << ") / (" << dms.finalStartX
                         << ", " << dms.finalStartY
                         << ")-(" << dms.finalEndX
                         << ", " << dms.finalEndY
                         << ") incorrectly degenerated "
                         << "in initial instant"
                         << endl;

                    delete uregion;
                    correct = false;
                    return SetWord(Address(0));
                }
            } else
                dms.degeneratedInitial = DGM_NONE;
        }

        if (dms.degeneratedFinal == DGM_UNKNOWN) {
            if (dms.degeneratedFinalNext >= 0) {
                const MSegmentData *auxDegenDms;
                MSegmentData degenDms;
                unsigned int numInsideAbove = 0;
                unsigned int numNotInsideAbove = 0;
                for (int j = i+1;
                     j != 0;
                     j = degenDms.degeneratedFinalNext) {
                    uregion->GetSegment(j-1, auxDegenDms);
                    degenDms = *auxDegenDms;
                    if (MRA_DEBUG)
                        cerr << "InURegion() degen-magic-f "
                             << i
                             << " "
                             << j-1
                             << endl;
                    if (degenDms.insideAbove)
                        numInsideAbove++;
                    else
                        numNotInsideAbove++;
                    if (j != i+1) {
                        degenDms.degeneratedFinal = DGM_IGNORE;
                        uregion->PutSegment(j-1, degenDms);
                    }
                }

                if (MRA_DEBUG)
                    cerr << "InURegion() degen-magic-f result "
                         << numInsideAbove
                         << " "
                         << numNotInsideAbove
                         << endl;

                if (numInsideAbove == numNotInsideAbove) {
                    dms.degeneratedFinal = DGM_IGNORE;
                } else if (numInsideAbove == numNotInsideAbove+1) {
                    dms.degeneratedFinal = DGM_INSIDEABOVE;
                } else if (numInsideAbove+1 == numNotInsideAbove) {
                    dms.degeneratedFinal = DGM_NOTINSIDEABOVE;
                } else {
                    cerr << "segment ("
                         << dms.initialStartX
                         << ", " << dms.initialStartY
                         << ")-(" << dms.initialEndX
                         << ", " << dms.initialEndY
                         << ") / (" << dms.finalStartX
                         << ", " << dms.finalStartY
                         << ")-(" << dms.finalEndX
                         << ", " << dms.finalEndY
                         << ") incorrectly degenerated "
                         << "in final instant"
                         << endl;

                    delete uregion;
                    correct = false;
                    return SetWord(Address(0));
                }
            } else
                dms.degeneratedFinal = DGM_NONE;
        }

        uregion->PutSegment(i, dms);
    }

#ifdef SCHMUH
    if (lc && !nonTrivialInitial) {
        cerr << "no non-trivial segments in initial instant but "
             << "time interval closed on left side"
             << endl;
        delete uregion;
        correct = false;
        return SetWord(Address(0));
    }

    if (rc && !nonTrivialFinal) {
        cerr << "no non-trivial segments in final instant but "
             << "time interval closed on right side"
             << endl;
        delete uregion;
        correct = false;
        return SetWord(Address(0));
    }
#endif

   if (MRA_DEBUG)
        for (int i = 0; i < uregion->GetSegmentsNum(); i++) {
            const MSegmentData *dms;

            uregion->GetSegment(i, dms);

            cerr << "InURegion() resulting segment #"
                 << i
                 << ": "
                 << dms->faceno
                 << " "
                 << dms->cycleno
                 << " "
                 << dms->segmentno
                 << " i=("
                 << dms->initialStartX
                 << ", "
                 << dms->initialStartY
                 << ", "
                 << dms->initialEndX
                 << ", "
                 << dms->initialEndY
                 << ") f=("
                 << dms->finalStartX
                 << ", "
                 << dms->finalStartY
                 << ", "
                 << dms->finalEndX
                 << ", "
                 << dms->finalEndY
                 << ") flags="
                 << dms->insideAbove
                 << " "
                 << dms->pointInitial
                 << " "
                 << dms->pointFinal
                 << " "
                 << dms->degeneratedInitialNext
                 << " "
                 << dms->degeneratedFinalNext
                 << endl;
        }

    correct = true;
    return SetWord(Address(uregion));
}

/*
1.1.1 Function ~InURegion()~

*/

static Word InURegion(const ListExpr typeInfo,
                      const ListExpr instance,
                      const int errorPos,
                      ListExpr& errorInfo,
                      bool& correct) {
    if (MRA_DEBUG) cerr << "InURegion() called" << endl;

    assert(false);

    return
        InURegionEmbedded(typeInfo,
                          instance,
                          errorPos,
                          errorInfo,
                          correct,
                          0,
                          0);
}

/*
1.1.1 Function ~CreateURegion()~

*/

static Word CreateURegion(const ListExpr typeInfo) {
    if (MRA_DEBUG) cerr << "CreateURegion() called" << endl;

    assert(false);
}

/*
1.1.1 Function ~DeleteURegion()~

*/
static void DeleteURegion(const ListExpr typeInfo, Word& w) {
    if (MRA_DEBUG) cerr << "DeleteURegion() called" << endl;

    URegion *ur = (URegion *)w.addr;
    delete ur;
    w.addr = 0;
}

/*
1.1.1 Function ~CloseURegion()~

*/
static void CloseURegion(const ListExpr typeInfo, Word& w) {
    if (MRA_DEBUG) cerr << "CloseURegion() called" << endl;

    delete (URegion *) w.addr;
    w.addr = 0;
}

/*
1.1.1 Function ~CloneURegion()~

*/
static Word CloneURegion(const ListExpr typeInfo, const Word& w) {
    if (MRA_DEBUG) cerr << "CloneURegion() called" << endl;

    assert(false);
}

/*
1.1.1 Function ~CastURegion()~

*/
static void* CastURegion(void* addr) {
    if (MRA_DEBUG) cerr << "CastURegion() called" << endl;

    assert(false);
}

/*
1.1.1 Function ~SizeOfURegion()~

*/
static int SizeOfURegion() {
    if (MRA_DEBUG) cerr << "SizeOfURegion() called" << endl;

    return 0;
}

/*
1.1.1 Type constructor ~uregion~

*/
static TypeConstructor uregion(
    "uregion",
    URegionProperty,
    OutURegion,
    InURegion,
    0, 0, // SaveToList, RestoreFromList
    CreateURegion,
    DeleteURegion,
    0, 0, // open, save
    CloseURegion,
    CloneURegion,
    CastURegion,
    SizeOfURegion,
    CheckURegion );

/*
1 Data type ~movingregion~

1.1 Class ~MRegion~

1.1.1 Class definition

Represents a moving region. It contains an array of segments, which is
references by its ~URegion~ units, which do not have its own storage for
segments.

*/

class MRegion : public Mapping<URegion, CRegion> {
private:
/*
The array with the segments.

*/
    DBArray<MSegmentData> msegmentdata;

/*
Calculates the intersection between this ~mp~ instance and ~mp~ based
on the refinement partition ~rp~. The result goes into ~res~.

*/
    void IntersectionRP(
        MPoint& mp,
        MPoint& res,
        RefinementPartition<MRegion, MPoint, URegion, UPoint>& rp);

/*
Add a ~UBool~ unit to ~res~ for interval $(starttime, endtime, lc, rc)$
with value ~value~. ~pending~ is used to merge units to reduce their
number. ~prev~ and ~prev\_c~ are used to assure continuous coverage.

*/
    void InsideAddUBool(MBool& res,
                        double starttime,
                        double endtime,
                        bool lc,
                        bool rc,
                        bool value,
                        double& prev,
                        bool& prev_c,
                        UBool*& pending);

public:
    MRegion() {
        if (MRA_DEBUG)
            cerr << "MRegion::MRegion(int) called" << endl;
    }

/*
Create ~MRegion()~ instance, which is prepared for ~n~ units.

*/
    MRegion(const int n) :
        Mapping<URegion, CRegion>(n),
        msegmentdata(0) {

        if (MRA_DEBUG)
            cerr << "MRegion::MRegion(int) called" << endl;
    }

/*
Create ~MRegion()~ instance, determine its units by the units in ~mp~ and
set each unit to the constant value of ~r~.

*/
    MRegion(MPoint& mp, CRegion& r) :
        Mapping<URegion, CRegion>(0),
        msegmentdata(0) {

        if (MRA_DEBUG)
            cerr << "MRegion::MRegion(MPoint, CRegion) called"
                 << endl;

        r.logicsort();

        for (int i = 0; i < mp.GetNoComponents(); i++) {
            const UPoint *up;

            mp.Get(i, up);

            if (MRA_DEBUG)
                cerr << "MRegion::MRegion(MPoint, CRegion) i="
                     << i
                     << " interval=["
                     << up->timeInterval.start.ToString()
                     << " ("
                     << up->timeInterval.start.ToDouble()
                     << ") "
                     << up->timeInterval.end.ToString()
                     << "("
                     << up->timeInterval.end.ToDouble()
                     << ") "
                     << up->timeInterval.lc
                     << " "
                     << up->timeInterval.rc
                     << "]"
                     << endl;

            URegion
                ur(up->timeInterval,
                   r,
                   &msegmentdata,
                   msegmentdata.Size());
            Add(ur);
        }
    }

/*
~DBArray~ access.

*/
    int NumOfFLOBs();
    FLOB *GetFLOB(const int i);

/*
Calculate intersection between ~mp~ and this ~MRegion~ isntance and
return result in ~res~.

*/
    void Intersection(MPoint& mp, MPoint& res);

/*
Check when ~mp~ is inside this ~MRegion~ instance and create ~res~
accordingly.

*/
    void Inside(MPoint& mp, MBool& res);

/*
Calculate region traversed by ~MRegion~ instant in ~res~.

*/
    void Traversed(CRegion& res);

/*
Get ~CRegion~ value ~result~ at instant ~t~.

*/
    void AtInstant(Instant& t, Intime<CRegion>& result);

/*
Get ~CRegion~ value ~result~ at initial and final instants.

*/
    void Initial(Intime<CRegion>& result);
    void Final(Intime<CRegion>& result);

/*
Friend access for ~InMRegion()~ and ~OpenMRegion()~ makes live easier.

*/
    friend Word InMRegion(const ListExpr typeInfo,
                          const ListExpr instance,
                          const int errorPos,
                          ListExpr& errorInfo,
                          bool& correct);

/*
Get ~URegion~ unit ~i~ from this ~MRegion~ instance and return it in ~ur~.

*/
    void Get(const int i, URegion& ur) {
        if (MRA_DEBUG)
            cerr << "MRegion::Get() called i=" << i << endl;

        const URegion *auxUr;
        Mapping<URegion, CRegion>::Get(i, auxUr);
        ur = *auxUr;
        ur.SetMSegmentData(&msegmentdata);
    }

/*
For unit testing only.

*/

    int Unittest2(int pos) {
        if (MRA_DEBUG)
            cerr << "MRegion::Unittest2() called pos="
                 << pos
                 << endl;

        if (pos < 0 || pos >= msegmentdata.Size()) return -1;

        const MSegmentData *dms;
        msegmentdata.Get(pos, dms);

        return dms->GetInsideAbove() ? 1 : 0;
    }
};

/*
1.1.1 ~DBArray~ access

*/

int MRegion::NumOfFLOBs() {
    if (MRA_DEBUG) cerr << "MRegion::NumOfFLOBs() called" << endl;

    return 2;
}

FLOB* MRegion::GetFLOB(const int i) {
    if (MRA_DEBUG) cerr << "MRegion::GetFLOB() called" << endl;

    assert(i == 0 || i == 1);

    return
        i == 0
        ? Mapping<URegion, CRegion>::GetFLOB(0)
        : &msegmentdata;
}

/*
1.1.1 Method ~Intersection()~

*/
void MRegion::IntersectionRP(
    MPoint& mp,
    MPoint& res,
    RefinementPartition<
        MRegion,
        MPoint,
        URegion,
        UPoint>& rp) {

    if (MRA_DEBUG)
        cerr << "MRegion::IntersectionRP() #1 called" << endl;

    res = 0;

    UPoint* pending = 0;

/*

For each interval in the refinement partition, we have to check whether
it maps to a region and point unit. If not, there is obviously no intersection
during this interval and we can skip if. Otherwise, we check if the region
and point unit, both restricted to this interval, intersect.

*/
    for (unsigned int i = 0; i < rp.Size(); i++) {
        Interval<Instant>* iv;
        int urPos;
        int upPos;

        rp.Get(i, iv, urPos, upPos);

        if (MRA_DEBUG)
            cerr << "MRegion::IntersectionRP() interval#"
                 << i
                 << ": "
                 << iv->start.ToDouble()
                 << " "
                 << iv->end.ToDouble()
                 << " "
                 << iv->lc
                 << " "
                 << iv->rc
                 << " "
                 << urPos
                 << " "
                 << upPos
                 << endl;

        if (urPos == -1 || upPos == -1) continue;

        URegion ur;
        const UPoint *up;

        Get(urPos, ur);
        mp.Get(upPos, up);

        if (MRA_DEBUG)
            cerr << "MRegion::IntersectionRP() both elements present"
                 << endl;

        ur.RestrictedIntersection(*up, *iv, res, pending);
    }

    if (pending) {
        if (!(nearlyEqual(pending->timeInterval.start.ToDouble(),
                          pending->timeInterval.end.ToDouble())
              && (!pending->timeInterval.lc
                  || !pending->timeInterval.rc)))
            res.Add(*pending);

        delete pending;
    }

    if (MRA_DEBUG)
        cerr << "MRegion::IntersectionRP() res.IsEmpty()="
             << res.IsEmpty()
             << endl;

    res.SetDefined(!res.IsEmpty());

}

/*
1.1.1 Method ~Intersection()~

*/
void MRegion::Intersection(MPoint& mp, MPoint& res) {
    if (MRA_DEBUG)
        cerr << "MRegion::Intersection() #2 called" << endl;

    RefinementPartition<
        MRegion,
        MPoint,
        URegion,
        UPoint> rp(*this, mp);

    IntersectionRP(mp, res, rp);
}

/*
1.1.1 Method ~InsideAddUBool()~

*/
void MRegion::InsideAddUBool(MBool& res,
                             double starttime,
                             double endtime,
                             bool lc,
                             bool rc,
                             bool value,
                             double& prev,
                             bool& prev_c,
                             UBool*& pending) {
    if (MRA_DEBUG)
        cerr << "MRegion::InsideAddUBool() called, value="
             << value
             << endl;

    Instant start(instanttype);
    start.ReadFrom(starttime);

    Instant end(instanttype);
    end.ReadFrom(endtime);

    CcBool bv(true, value);

    if (pending) {
        if (MRA_DEBUG) {
            cerr << "MRegion::InsideAddUBool() pending end="
                 << pending->timeInterval.end.ToDouble()
                 << " rc="
                 << pending->timeInterval.rc
                 << endl;
            cerr << "MRegion::InsideAddUBool() current start="
                 << starttime
                 << " lc="
                 << lc
                 << endl;
        }

        if (nearlyEqual(starttime,
                        pending->timeInterval.end.ToDouble())
            && (pending->timeInterval.rc || lc)
            && pending->constValue.GetBoolval() == value) {

            if (MRA_DEBUG)
                cerr << "MRegion::InsideAddUBool() connecting"
                     << endl;

            if (nearlyEqual(pending->timeInterval.start.ToDouble(),
                            pending->timeInterval.end.ToDouble())) {
                Interval<Instant> iv(start,
                                     end,
                                     true,
                                     rc);

                delete pending;
                pending = new UBool(iv, bv);
            } else if (nearlyEqual(starttime, endtime)) {
                Interval<Instant> iv(pending->timeInterval.start,
                                     pending->timeInterval.end,
                                     pending->timeInterval.lc,
                                     true);

                delete pending;
                pending = new UBool(iv, bv);
            } else {
                Interval<Instant> iv(pending->timeInterval.start,
                                     end,
                                     pending->timeInterval.lc,
                                     rc);

                delete pending;
                pending = new UBool(iv, bv);
            }
        } else {
            if (MRA_DEBUG)
                cerr << "MRegion::InsideAddUBool() not connecting"
                     << endl;

            res.Add(*pending);
            delete pending;

            Interval<Instant> iv(start, end, lc, rc);

            pending = new UBool(iv, bv);
        }
    } else {
        if (MRA_DEBUG)
            cerr << "MRegion::InsideAddUBool() "
                 << "pending does not exist"
                 << endl;

        Interval<Instant> iv(start, end, lc, rc);

        pending = new UBool(iv, bv);
    }

    prev = endtime;
    prev_c = rc;
}

/*
1.1.1 Method ~Inside()~

*/
void MRegion::Inside(MPoint& mp, MBool& res) {
    if (MRA_DEBUG) cerr << "MRegion::Inside() called" << endl;

/*
Use intersection algorithm and then see how the units in the result
match to the original units in ~mp~.

*/
    RefinementPartition<
        MRegion,
        MPoint,
        URegion,
        UPoint> rp(*this, mp);

    MPoint resMp;
    IntersectionRP(mp, resMp, rp);

    int mpPos = 0;

    UBool* pending = 0;

    for (unsigned int rpPos = 0; rpPos < rp.Size(); rpPos++) {
        if (MRA_DEBUG)
            cerr << "MRegion::Inside() rpPos=" << rpPos << endl;

        Interval<Instant>* iv;
        int urPos;
        int upPos;

        rp.Get(rpPos, iv, urPos, upPos);

        double prev = iv->start.ToDouble();
        bool prev_c = !iv->lc;

        for (; mpPos < resMp.GetNoComponents(); mpPos++) {
            if (MRA_DEBUG)
                cerr << "MRegion::Inside()   mpPos=" << mpPos
                     << endl;

            const UPoint *up;
            resMp.Get(mpPos, up);

            if (MRA_DEBUG)
                cerr << "MRegion::Inside()     rp iv=["
                     << iv->start.ToDouble()
                     << " "
                     << iv->end.ToDouble()
                     << " "
                     << iv->lc
                     << " "
                     << iv->rc
                     << "] up iv=["
                     << up->timeInterval.start.ToDouble()
                     << " "
                     << up->timeInterval.end.ToDouble()
                     << " "
                     << up->timeInterval.lc
                     << " "
                     << up->timeInterval.rc
                     << "] prev="
                     << prev
                     << endl;

            if ((lower(iv->start.ToDouble(),
                       up->timeInterval.start.ToDouble())
                 || (nearlyEqual(iv->start.ToDouble(),
                                 up->timeInterval.start.ToDouble())
                     && (iv->lc || !up->timeInterval.lc)))
                && (lower(up->timeInterval.end.ToDouble(),
                          iv->end.ToDouble())
                    || (nearlyEqual(up->timeInterval.end.ToDouble(),
                                    iv->end.ToDouble())
                        && (!up->timeInterval.rc || iv->rc)))) {

                if (MRA_DEBUG)
                    cerr << "MRegion::Inside()     inside" << endl;

                if (lower(prev, up->timeInterval.start.ToDouble())
                    || (nearlyEqual(prev,
                                    up->timeInterval.start.ToDouble())
                        && !prev_c
                        && !up->timeInterval.lc)) {
                    if (MRA_DEBUG)
                        cerr << "MRegion::Inside()     "
                             << "adding f for interval ["
                             << prev
                             << " "
                             << up->timeInterval.start.ToDouble()
                             << " "
                             << !prev_c
                             << " "
                             << !up->timeInterval.lc
                             << "]"
                             << endl;

                    InsideAddUBool(res,
                                   prev,
                                   up->timeInterval.start.ToDouble(),
                                   !prev_c,
                                   !up->timeInterval.lc,
                                   false,
                                   prev,
                                   prev_c,
                                   pending);
                }

                if (MRA_DEBUG)
                    cerr << "MRegion::Inside()     "
                         << "adding t for interval ["
                         << up->timeInterval.start.ToDouble()
                         << " "
                         << up->timeInterval.end.ToDouble()
                         << " "
                         << up->timeInterval.lc
                         << " "
                         << up->timeInterval.rc
                         << "]"
                         << endl;

                InsideAddUBool(res,
                               up->timeInterval.start.ToDouble(),
                               up->timeInterval.end.ToDouble(),
                               up->timeInterval.lc,
                               up->timeInterval.rc,
                               true,
                               prev,
                               prev_c,
                               pending);
            } else {
                if (MRA_DEBUG)
                    cerr << "MRegion::Inside()     "
                         << "not inside"
                         << endl;

                break;
            }
        }

        if (lower(prev, iv->end.ToDouble())
            || (nearlyEqual(prev, iv->end.ToDouble())
                && !prev_c
                && iv->rc)) {
            if (MRA_DEBUG)
                cerr << "MRegion::Inside()     "
                     << "adding f for interval ["
                     << prev
                     << " "
                     << iv->end.ToDouble()
                     << " "
                     << !prev_c
                     << " "
                     << iv->rc
                     << "]"
                     << endl;

            InsideAddUBool(res,
                           prev,
                           iv->end.ToDouble(),
                           !prev_c,
                           iv->rc,
                           false,
                           prev,
                           prev_c,
                           pending);
        }
    }

    if (pending) {
        res.Add(*pending);
        delete pending;
    }

    res.SetDefined(!res.IsEmpty());
}

/*
1.1.1 Method ~AtInstant()~

Method ~Mapping<Unit, Alpha>::AtInstant()~ is not sufficient since it
does not deal with setting the unit's segment data DBArray.

*/

void MRegion::AtInstant(Instant& t, Intime<CRegion>& result) {
    if (MRA_DEBUG) cerr << "MRegion::AtInstant() called" << endl;

    assert(IsOrdered() && t.IsDefined());

    int pos = Position(t );

    if( pos == -1 )
        result.SetDefined(false);
    else {
        URegion posUnit;
        Get(pos, posUnit);

        result.SetDefined(true);
        posUnit.TemporalFunction(t, result.value);
        result.instant.CopyFrom(&t);
  }
}

/*
1.1.1 Method ~Initial()~

Method ~Mapping<Unit, Alpha>::Initial()~ is not sufficient since it
does not deal with setting the unit's segment data DBArray.

*/

void MRegion::Initial(Intime<CRegion>& result) {
    if (MRA_DEBUG) cerr << "MRegion::Initial() called" << endl;

    assert(IsOrdered());

    if (IsEmpty()) {
        result.SetDefined(false);
        return;
    }

    URegion unit;
    Get(0, unit);

    if (!unit.timeInterval.lc) {
        result.SetDefined(false);
        return;
    }
        
    result.SetDefined(true);
    unit.TemporalFunction(unit.timeInterval.start, result.value);
    result.instant.CopyFrom(&unit.timeInterval.start);
}

/*
1.1.1 Method ~Final()~

Method ~Mapping<Unit, Alpha>::Final()~ is not sufficient since it
does not deal with setting the unit's segment data DBArray.

*/

void MRegion::Final(Intime<CRegion>& result) {
    if (MRA_DEBUG) cerr << "MRegion::Final() called" << endl;

    assert(IsOrdered());

    if (IsEmpty()) {
        result.SetDefined(false);
        return;
    }

    URegion unit;
    Get(GetNoComponents()-1, unit);
        
    if (!unit.timeInterval.rc) {
        result.SetDefined(false);
        return;
    }
        
    result.SetDefined(true);
    unit.TemporalFunction(unit.timeInterval.end, result.value);
    result.instant.CopyFrom(&unit.timeInterval.end);
}

/*
1.1.1 Method ~Traversed()~

*/

void MRegion::Traversed(CRegion& res) {
    if (MRA_DEBUG) cerr << "MRegion::Traversed() called" << endl;

    assert(false);
}

/*
1.1 Algebra integration

1.1.1 Function ~MRegionProperty()~

*/

// *hm* Verify list representation

static ListExpr MRegionProperty() {
    if (MRA_DEBUG) cerr << "MRegionProperty() called" << endl;

    ListExpr listrep = nl->TextAtom();
    nl->AppendText(listrep,
                   "(u1 ... un) with ui uregion list representations "
                   "and n >= 1.");
    ListExpr example = nl->TextAtom();
    nl->AppendText(example,
                   "(((0.0 10.0 TRUE TRUE)"
                   "((((1.0 3.5 1.5 1.5)"
                   "(2.0 5.5 3.0 4.5)"
                   "(3.0 6.5 3.5 5.0)"
                   "(4.0 6.5 5.5 5.0)"
                   "(4.0 5.5 5.5 4.5)"
                   "(5.0 4.5 7.5 2.5)"
                   "(5.0 2.5 7.5 1.0)"
                   "(4.0 1.5 7.0 0.5)"
                   "(3.0 1.5 2.5 0.5))"
                   "((2.0 3.0 3.0 2.0)"
                   "(2.0 4.0 3.0 3.0)"
                   "(3.0 4.0 4.0 3.0)"
                   "(3.0 3.0 4.0 2.0))))))");

    return
        nl->TwoElemList(
            nl->FourElemList(
                nl->StringAtom("Signature"),
                nl->StringAtom("Example Type List"),
                nl->StringAtom("List Rep"),
                nl->StringAtom("Example List")),
            nl->FourElemList(
                nl->StringAtom("-> MAPPING"),
                nl->StringAtom("(movingregion)"),
                listrep,
                example));
}

/*
1.1.1 Function ~CheckMRegion()~

*/

static bool CheckMRegion(ListExpr type, ListExpr& errorInfo) {
    if (MRA_DEBUG) cerr << "CheckMRegion() called" << endl;

    return nl->IsEqual(type, "movingregion");
}

/*
1.1.1 Function ~OutMRegion()~

*/

static ListExpr OutMRegion(ListExpr typeInfo, Word value) {
    if (MRA_DEBUG) cerr << "OutMRegion() called" << endl;

    MRegion* mr = (MRegion*) value.addr;

    if (mr->IsEmpty()) return (nl->TheEmptyList());

    assert(mr->IsOrdered());

    ListExpr l = nl->TheEmptyList();
    ListExpr lastElem;

    for(int i = 0; i < mr->GetNoComponents(); i++) {
        URegion ur;
        mr->Get(i, ur);
        ListExpr unitList = OutURegion(nl->TheEmptyList(), SetWord(&ur));

        if (l == nl->TheEmptyList()) {
            l = nl->Cons(unitList, nl->TheEmptyList());
            lastElem = l;
        } else
            lastElem = nl->Append(lastElem, unitList);
    }
    return l;
}

/*
1.1.1 Function ~InMRegion()~

*/
Word InMRegion(const ListExpr typeInfo,
                      const ListExpr instance,
                      const int errorPos,
                      ListExpr& errorInfo,
                      bool& correct) {
    if (MRA_DEBUG) cerr << "InMRegion() called" << endl;

    MRegion* mr = new MRegion(0);

    mr->StartBulkLoad();

    ListExpr rest = instance;
    while(!nl->IsEmpty(rest)) {
        ListExpr first = nl->First(rest);
        rest = nl->Rest(rest);

        URegion* ur =
            (URegion*) InURegionEmbedded(
                nl->TheEmptyList(),
                first,
                errorPos,
                errorInfo,
                correct,
                &mr->msegmentdata,
                mr->msegmentdata.Size()).addr;
        if(correct == false) return SetWord(Address(0));
        mr->Add(*ur);
        delete ur;
    }

    mr->EndBulkLoad(true);

    if (mr->IsValid()) {
        correct = true;
        return SetWord(mr);
    } else {
        correct = false;
        mr->Destroy();
        delete mr;
        return SetWord(0);
    }
}

/*
1.1.1 Function ~OpenMRegion()~

*/
bool OpenMRegion(SmiRecord& rec,
                 size_t& offset,
                 const ListExpr typeInfo,
                 Word& w) {
    if (MRA_DEBUG) cerr << "OpenMRegion() called" << endl;

    MRegion* mr = (MRegion*) TupleElement::Open(rec, offset, typeInfo);
    w = SetWord(mr);

    return true;
}

/*
1.1.1 Function ~SaveMRegion()~

*/
static bool SaveMRegion(SmiRecord& rec,
                        size_t& offset,
                        const ListExpr typeInfo,
                        Word& w) {
    if (MRA_DEBUG) cerr << "SaveMRegion() called" << endl;

    MRegion* mr = (MRegion*) w.addr;
    TupleElement::Save(rec, offset, typeInfo, mr);
    return true;
}

/*
1.1.1 Type constructor ~mregion~

*/
static TypeConstructor mregion(
    "movingregion",
    MRegionProperty,
    OutMRegion,
    InMRegion,
    0, 0, // SaveToList, RestoreFromList
    CreateMapping<MRegion>, DeleteMapping<MRegion>,
    OpenMRegion, SaveMRegion,
    CloseMapping<MRegion>, CloneMapping<MRegion>,
    CastMapping<MRegion>,
    SizeOfMapping<MRegion>,
    CheckMRegion );

/*
1.1 Helper function(s)

Create ~MPoint~ instance from intervals in units in ~MRegion~ instance
~mr~ and the constant point ~p~.

*/

static MPoint CreateMPointFromCPoint(MRegion* mr, CPoint* p) {
    if (MRA_DEBUG) cerr << "CreateMPointFromCPoint() called" << endl;

    MPoint mp(0);

    for (int i = 0; i < mr->GetNoComponents(); i++) {
        if (MRA_DEBUG) cerr << "CreateMPointFromCPoint() i=" << i << endl;

        URegion ur;

        mr->Get(i, ur);

        if (MRA_DEBUG) cerr << "CreateMPointFromCPoint() trace #1" << endl;

        UPoint up(ur.timeInterval,
                  p->GetX(), p->GetY(),
                  p->GetX(), p->GetY());

        if (MRA_DEBUG) cerr << "CreateMPointFromCPoint() trace #2" << endl;

        mp.Add(up);

        if (MRA_DEBUG) cerr << "CreateMPointFromCPoint() trace #3" << endl;
    }

    return mp;
}

/*
1 Operator definition

1.1 Type mapping functions

1.1.1 Generic

Used by ~intersection~:

*/

static ListExpr MPointMRegionToMPointTypeMap(ListExpr args) {
    if (MRA_DEBUG)
        cerr << "MPointMRegionToMPointTypeMap() called" << endl;

    cerr << nl->SymbolValue(nl->First(args)) << endl;
    cerr << nl->SymbolValue(nl->Second(args)) << endl;

    if (nl->ListLength(args) == 2
        && nl->IsEqual(nl->First(args), "mpoint")
        && nl->IsEqual(nl->Second(args), "movingregion"))
        return nl->SymbolAtom("mpoint");
    else
        return nl->SymbolAtom("typeerror");
}

/*
Used by ~inside~:

*/

static ListExpr MPointMRegionToMBoolTypeMap(ListExpr args) {
    if (MRA_DEBUG)
        cerr << "MPointMRegionToMBoolTypeMap() called" << endl;

    if (nl->ListLength(args) == 2
        && nl->IsEqual(nl->First(args), "mpoint")
        && nl->IsEqual(nl->Second(args), "movingregion"))
        return nl->SymbolAtom("mbool");
    else
        return nl->SymbolAtom("typeerror");
}

/*
Used by ~atinstant~:

*/
static ListExpr MRegionInstantToIRegionTypeMap(ListExpr args) {
    if (MRA_DEBUG)
        cerr << "MRegionInstantToIRegionTypeMap() called" << endl;

    if (nl->ListLength(args) == 2
        && nl->IsEqual(nl->First(args), "movingregion")
        && nl->IsEqual(nl->Second(args), "instant"))
        return nl->SymbolAtom("intimeregion");
    else
        return nl->SymbolAtom("typeerror");
}

/*
Used by ~initial~ and ~final~:

*/
static ListExpr MRegionToIRegionTypeMap(ListExpr args) {
    if (MRA_DEBUG)
        cerr << "MRegionToIRegionTypeMap() called" << endl;

    if (nl->ListLength(args) == 1
        && nl->IsEqual(nl->First(args), "movingregion"))
        return nl->SymbolAtom("intimeregion");
    else
        return nl->SymbolAtom("typeerror");
}

/*
Used by ~deftime~:

*/
static ListExpr MRegionToPeriodsTypeMap(ListExpr args) {
    if (MRA_DEBUG)
        cerr << "MRegionToPeriodsTypeMap() called" << endl;

    if (nl->ListLength(args) == 1
        && nl->IsEqual(nl->First(args), "movingregion"))
        return nl->SymbolAtom("periods");
    else
        return nl->SymbolAtom("typeerror");
}

/*
Used by ~inst~:

*/

static ListExpr IRegionToInstantTypeMap(ListExpr args) {
    if (MRA_DEBUG)
        cerr << "IRegionToInstantTypeMap() called" << endl;

    if (nl->ListLength(args) == 1
        && nl->IsEqual(nl->First(args), "intimeregion"))
        return nl->SymbolAtom("instant");
    else
        return nl->SymbolAtom("typeerror");
}

/*
Used by ~val~:

*/

static ListExpr IRegionToRegionTypeMap(ListExpr args) {
    if (MRA_DEBUG)
        cerr << "IRegionToRegionTypeMap() called" << endl;

    if (nl->ListLength(args) == 1
        && nl->IsEqual(nl->First(args), "intimeregion"))
        return nl->SymbolAtom("region");
    else
        return nl->SymbolAtom("typeerror");
}

/*
Used by ~traversed~:

*/
static ListExpr MRegionToRegionTypeMap(ListExpr args) {
    if (MRA_DEBUG)
        cerr << "MRegionToRegionTypeMap() called" << endl;

    if (nl->ListLength(args) == 1
        && nl->IsEqual(nl->First(args), "movingregion"))
        return nl->SymbolAtom("region");
    else
        return nl->SymbolAtom("typeerror");
}

/*

1.1.1 Operator specific

Used by ~present~:

*/
static ListExpr PresentTypeMap(ListExpr args) {
    if (MRA_DEBUG)
        cerr << "PresentTypeMap() called" << endl;

    if (nl->ListLength(args) == 2
        && nl->IsEqual(nl->First(args), "movingregion")
        && (nl->IsEqual(nl->Second(args), "instant")
            || nl->IsEqual(nl->Second(args), "periods")))
        return nl->SymbolAtom("bool");
    else
        return nl->SymbolAtom("typeerror");
}

/*
Used by ~at~:

*/

static ListExpr AtTypeMap(ListExpr args) {
    if (MRA_DEBUG)
        cerr << "AtTypeMap() called" << endl;

    if (nl->ListLength(args) == 2
        && nl->IsEqual(nl->First(args), "mpoint")
        && nl->IsEqual(nl->Second(args), "region"))
        return nl->SymbolAtom("mpoint");
    else if (nl->ListLength(args) == 2
        && nl->IsEqual(nl->First(args), "movingregion")
        && nl->IsEqual(nl->Second(args), "point"))
        return nl->SymbolAtom("mpoint");
    else
        return nl->SymbolAtom("typeerror");
}

/*
Used by ~mraprec~:

*/

static ListExpr MraprecTypeMap(ListExpr args) {
    if (MRA_DEBUG)
        cerr << "MraprecTypeMap() called" << endl;

    if (nl->ListLength(args) == 1
        && nl->IsEqual(nl->First(args), "real"))
        return nl->SymbolAtom("bool");
    else
        return nl->SymbolAtom("typeerror");
}

/*
1.1.1 For unit testing operators

*/

static ListExpr Unittest1TypeMap(ListExpr args) {
    if (MRA_DEBUG)
        cerr << "Unittest1TypeMap() called" << endl;

    if (nl->ListLength(args) != 17)
        return nl->SymbolAtom("typeerror");

    for (; !nl->IsEmpty(args); args = nl->Rest(args))
        if (nl->SymbolValue(nl->First(args)) != "real")
            return nl->SymbolAtom("typeerror");

    return nl->SymbolAtom("int");
}

static ListExpr Unittest2TypeMap(ListExpr args) {
    if (MRA_DEBUG)
        cerr << "Unittest2TypeMap() called" << endl;

    if (nl->ListLength(args) == 2
        && nl->IsEqual(nl->First(args), "movingregion")
        && nl->IsEqual(nl->Second(args), "int"))
        return nl->SymbolAtom("int");
    else
        return nl->SymbolAtom("typeerror");
}

/*
1.1 Selection functions

1.1.1 Generic

Used by ~intersection~ and ~inside~:

*/

static int MPointMRegionSelect(ListExpr args) {
    if (MRA_DEBUG)
        cerr << "MPointMRegionSelect() called" << endl;

    if (nl->ListLength(args) == 2
        && nl->SymbolValue(nl->First(args)) == "mpoint"
        && nl->SymbolValue(nl->Second(args)) == "movingregion")
        return 0;
    else
        return -1;
}

/*
Used by ~initial~, ~final~, ~deftime~ and ~traversed~:

*/

static int MRegionSelect(ListExpr args) {
    if (MRA_DEBUG)
        cerr << "MRegionSelect() called" << endl;

    if (nl->ListLength(args) == 1
        && nl->SymbolValue(nl->First(args)) == "movingregion")
        return 0;
    else
        return -1;
}

/*
Used by ~atinstant~:

*/

static int MRegionInstantSelect(ListExpr args) {
    if (MRA_DEBUG)
        cerr << "MRegionInstantSelect() called" << endl;

    if (nl->ListLength(args) == 2
        && nl->SymbolValue(nl->First(args)) == "movingregion"
        && nl->SymbolValue(nl->Second(args)) == "instant")
        return 0;
    else
        return -1;
}

/*
Used by ~inst~ and ~val~:

*/

static int IRegionSelect(ListExpr args) {
    if (MRA_DEBUG)
        cerr << "IRegionSelect() called" << endl;

    if (MRA_DEBUG)
        cerr << "IRegionSelect() len="
             << nl->ListLength(args)
             << endl;
    if (MRA_DEBUG)
        cerr << "IRegionSelect() symbolvalue(first)="
             << nl->SymbolValue(nl->First(args))
             << endl;

    if (nl->ListLength(args) == 1
        && nl->SymbolValue(nl->First(args)) == "intimeregion")
        return 0;
    else
        return -1;
}

/*
1.1.1 Operator specific

For ~present~:

*/

static int PresentSelect(ListExpr args) {
    if (MRA_DEBUG) cerr << "PresentSelect() called" << endl;

    if (nl->ListLength(args) == 2
        && nl->SymbolValue(nl->First(args)) == "movingregion"
        && nl->SymbolValue(nl->Second(args)) == "instant")
        return 0;
    else if (nl->ListLength(args) == 2
        && nl->SymbolValue(nl->First(args)) == "movingregion"
        && nl->SymbolValue(nl->Second(args)) == "periods")
        return 1;
    else
        return -1;
}

/*
For ~at~:

*/

static int AtSelect(ListExpr args) {
    if (MRA_DEBUG) cerr << "AtSelect() called" << endl;

    if (nl->ListLength(args) == 2
        && nl->SymbolValue(nl->First(args)) == "mpoint"
        && nl->SymbolValue(nl->Second(args)) == "region")
        return 0;
    else if (nl->ListLength(args) == 2
        && nl->SymbolValue(nl->First(args)) == "movingregion"
        && nl->SymbolValue(nl->Second(args)) == "point")
        return 1;
    else
        return -1;
}

/*
1.1 Value mapping functions

1.1.1 Normal value mapping functions

*/

static int IntersectionValueMap(Word* args,
                                Word& result,
                                int message,
                                Word& local,
                                Supplier s) {
    if (MRA_DEBUG) cerr << "IntersectionValueMap() called" << endl;

    result = qp->ResultStorage(s);
    ((MRegion*) args[1].addr)->Intersection(* (MPoint*) args[0].addr,
                                            * (MPoint*) result.addr);

    return 0;
}

static int InsideValueMap(Word* args,
                          Word& result,
                          int message,
                          Word& local,
                          Supplier s) {
    if (MRA_DEBUG) cerr << "InsideValueMap() called" << endl;

    result = qp->ResultStorage(s);
    ((MRegion*) args[1].addr)->Inside(* (MPoint*) args[0].addr,
                                      * (MBool*) result.addr);

    return 0;
}

static int AtValueMap_MPoint(Word* args,
                             Word& result,
                             int message,
                             Word& local,
                             Supplier s) {
    if (MRA_DEBUG) cerr << "AtValueMap_MPoint() called" << endl;

    MRegion* mr = new MRegion(* (MPoint*) args[0].addr,
                              * (CRegion*) args[1].addr);

    result = qp->ResultStorage(s);
    mr->Intersection(* (MPoint*) args[0].addr,
                     * (MPoint*) result.addr);

    return 0;
}

static int AtValueMap_MRegion(Word* args,
                              Word& result,
                              int message,
                              Word& local,
                              Supplier s) {
    if (MRA_DEBUG) cerr << "AtValueMap_MRegion() called" << endl;

    MRegion* mr = (MRegion*) args[0].addr;
    CPoint* p = (CPoint*) args[1].addr;

    MPoint mp = CreateMPointFromCPoint(mr, p);

    if (MRA_DEBUG) cerr << "AtValueMap_MRegion() trace #1" << endl;

    result = qp->ResultStorage(s);
    mr->Intersection(mp, * (MPoint*) result.addr);

    if (MRA_DEBUG) cerr << "AtValueMap_MRegion() trace #2" << endl;

    return 0;
}

static int TraversedValueMap(Word* args,
                             Word& result,
                             int message,
                             Word& local,
                             Supplier s) {
    if (MRA_DEBUG) cerr << "TraversedValueMap() called" << endl;

    result = qp->ResultStorage(s);

    ((MRegion*) args[0].addr)->Traversed(* (CRegion*) result.addr);

    return 0;
}


static int MraprecValueMap(Word* args,
                           Word& result,
                           int message,
                           Word& local,
                           Supplier s) {
    if (MRA_DEBUG) cerr << "MraprecValueMap() called" << endl;

    eps = ((CcReal*) args[0].addr)->GetRealval();

    result = qp->ResultStorage(s);
    ((CcBool*) result.addr)->Set(true, true);

    return 0;
}

/*
1.1.1 For unit testing operators

*/

static int Unittest1ValueMap(Word* args,
                             Word& result,
                             int message,
                             Word& local,
                             Supplier s) {
    if (MRA_DEBUG) cerr << "Unittest1ValueMap() called" << endl;

    unsigned int detailedResult;

    specialTrapeziumIntersects(
        ((CcReal*) args[0].addr)->GetRealval(),  // dt
        ((CcReal*) args[1].addr)->GetRealval(),  // t1p1x
        ((CcReal*) args[2].addr)->GetRealval(),  // t1p1y
        ((CcReal*) args[3].addr)->GetRealval(),  // t1p2x
        ((CcReal*) args[4].addr)->GetRealval(),  // t1p2y
        ((CcReal*) args[5].addr)->GetRealval(),  // t1p3x
        ((CcReal*) args[6].addr)->GetRealval(),  // t1p3y
        ((CcReal*) args[7].addr)->GetRealval(),  // t1p4x
        ((CcReal*) args[8].addr)->GetRealval(),  // t1p4y
        ((CcReal*) args[9].addr)->GetRealval(),  // t2p1x
        ((CcReal*) args[10].addr)->GetRealval(), // t2p1y
        ((CcReal*) args[11].addr)->GetRealval(), // t2p2x
        ((CcReal*) args[12].addr)->GetRealval(), // t2p2y
        ((CcReal*) args[13].addr)->GetRealval(), // t2p3x
        ((CcReal*) args[14].addr)->GetRealval(), // t2p3y
        ((CcReal*) args[15].addr)->GetRealval(), // t2p4x
        ((CcReal*) args[16].addr)->GetRealval(), // t2p4y
        detailedResult);

    if (MRA_DEBUG) cerr << "Unittest1ValueMap() #1" << endl;

    result = qp->ResultStorage(s);
    ((CcInt *)result.addr)->Set(true, detailedResult);

    if (MRA_DEBUG) cerr << "Unittest1ValueMap() #2" << endl;

    return 0;
}

static int Unittest2ValueMap(Word* args,
                             Word& result,
                             int message,
                             Word& local,
                             Supplier s) {
    if (MRA_DEBUG) cerr << "Unittest2ValueMap() called" << endl;

    result = qp->ResultStorage(s);
    ((CcInt *)result.addr)->Set(
        true,
        ((MRegion*) args[0].addr)->Unittest2(
            ((CcInt*) args[1].addr)->GetIntval()));

    return 0;
}


/*
1.1 Value mapping arrays

*/

// *hm* remove value mapping array from non-overloaded operators

static ValueMapping atinstantvaluemap[] =
    { MappingAtInstant<MRegion, CRegion> };

static ValueMapping initialvaluemap[] =
    { MappingInitial<MRegion, URegion, CRegion> };

static ValueMapping finalvaluemap[] =
    { MappingFinal<MRegion, URegion, CRegion> };

static ValueMapping instvaluemap[] =
    { IntimeInst<CRegion> };

static ValueMapping valvaluemap[] =
    { IntimeVal<CRegion> };

static ValueMapping deftimevaluemap[] =
    { MappingDefTime<MRegion> };

static ValueMapping presentvaluemap[] =
    { MappingPresent_i<MRegion>,
      MappingPresent_p<MRegion> };

static ValueMapping intersectionvaluemap[] =
    { IntersectionValueMap };

static ValueMapping insidevaluemap[] =
    { InsideValueMap };

static ValueMapping atvaluemap[] =
    { AtValueMap_MPoint,
      AtValueMap_MRegion };

static ValueMapping traversedvaluemap[] =
    { TraversedValueMap };

/*
1.1 Operator specifications

*/

static const string atinstantspec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "  ( <text>(mregion instant) -> iregion</text--->"
     "    <text>_ atinstant _ </text--->"
    "    <text>Get the iregion value corresponding "
    "to the instant.</text--->"
    "    <text>mregion1 atinstant instant1</text---> ) )";

static const string initialspec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "  ( <text>mregion -> iregion</text--->"
    "    <text>initial( _ )</text--->"
    "    <text>Get the iregion value corresponding to "
    "the initial instant."
    "    </text--->"
    "    <text>initial( mregion1 )</text---> ) )";

static const string finalspec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "  ( <text>mregion -> iregion</text--->"
    "    <text>final( _ )</text--->"
    "    <text>Get the iregion value corresponding to "
    "the final instant."
    "    </text--->"
    "    <text>final( mregion1 )</text---> ) )";

static const string instspec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "  ( <text>iregion -> instant</text--->"
    "    <text>inst ( _ )</text--->"
    "    <text>iregion time instant.</text--->"
    "    <text>inst ( iregion1 )</text---> ) )";

static const string valspec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "  ( <text>iregion -> region</text--->"
    "    <text>val ( _ )</text--->"
    "    <text>Intime value.</text--->"
    "    <text>val ( iregion1 )</text---> ) )";

static const string deftimespec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "  ( <text>mregion -> periods</text--->"
    "    <text>deftime( _ )</text--->"
    "    <text>Get the defined time of the mregion object.</text--->"
    "    <text>deftime( mregion1 )</text---> ) )";

static const string presentspec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "  ( <text>(mregion instant) -> bool, "
    "(mregion periods) -> bool</text--->"
    "    <text>_ present _ </text--->"
    "    <text>Whether the object is present at the given instant or"
    "    period.</text--->"
    "    <text>mregion1 present instant1</text---> ) )";

static const string intersectionspec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "  ( <text>(mpoint mregion) -> mpoint</text--->"
    "    <text>intersection( _ , _)</text--->"
    "    <text>Intersection between mpoint and mregion.</text--->"
    "    <text>intersection(mpoint1, mregion1)</text---> ) )";

static const string insidespec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "  ( <text>(mpoint mregion) -> mbool</text--->"
    "    <text>_ inside _</text--->"
    "    <text>Calculates if and when mpoint is inside "
    "mregion.</text--->"
    "    <text>mpoint1 inside mregion1</text---> ) )";

static const string atspec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "  ( <text>(mpoint region) -> mregion, "
    "(mregion point) -> mpoint</text--->"
    "    <text>_ at _</text--->"
    "    <text>Restrict moving point to region or restrict moving region "
    "to point.</text--->"
    "    <text>mpoint1 at region1</text---> ) )";

static const string traversedspec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "  ( <text>mregion -> region</text--->"
    "    <text>traversed( _ )</text--->"
    "    <text>Projection of a moving region into "
    "the plane.</text--->"
    "    <text>traversed(mregion1)</text---> ) )";

static const string mraprecspec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "  ( <text>(real) -> bool</text--->"
    "    <text>mraprec ( _ )</text--->"
    "    <text>Sets precision of comparisions. Always returns true.</text--->"
    "    <text>mraprec(0.0001)</text---> ) )";

/*
Used for unit testing only.

*/

static const string unittestspec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "  ( <text>unit testing only</text--->"
    "    <text>unit testing only</text--->"
    "    <text>unit testing only</text--->"
    "    <text>unit testing only</text---> ) )";

/*
1.1 Operator creation

*/

static Operator atinstant("atinstant",
                          atinstantspec,
                          1,
                          atinstantvaluemap,
                          MRegionInstantSelect,
                          MRegionInstantToIRegionTypeMap);

static Operator initial("initial",
                        initialspec,
                        1,
                        initialvaluemap,
                        MRegionSelect,
                        MRegionToIRegionTypeMap);

static Operator final("final",
                      finalspec,
                      1,
                      finalvaluemap,
                      MRegionSelect,
                      MRegionToIRegionTypeMap);

static Operator inst("inst",
                     instspec,
                     1,
                     instvaluemap,
                     IRegionSelect,
                     IRegionToInstantTypeMap);

static Operator val("val",
                    valspec,
                    1,
                    valvaluemap,
                    IRegionSelect,
                    IRegionToRegionTypeMap);

static Operator deftime("deftime",
                        deftimespec,
                        1,
                        deftimevaluemap,
                        MRegionSelect,
                        MRegionToPeriodsTypeMap);

static Operator present("present",
                        presentspec,
                        2,
                        presentvaluemap,
                        PresentSelect,
                        PresentTypeMap);

static Operator intersection("intersection",
                             intersectionspec,
                             1,
                             intersectionvaluemap,
                             MPointMRegionSelect,
                             MPointMRegionToMPointTypeMap);

static Operator inside("inside",
                       insidespec,
                       1,
                       insidevaluemap,
                       MPointMRegionSelect,
                       MPointMRegionToMBoolTypeMap);

static Operator at("at",
                   atspec,
                   2,
                   atvaluemap,
                   AtSelect,
                   AtTypeMap);

static Operator traversed("traversed",
                          traversedspec,
                          1,
                          traversedvaluemap,
                          MRegionSelect,
                          MRegionToRegionTypeMap);

static Operator mraprec("mraprec",
                        mraprecspec,
                        MraprecValueMap,
                        simpleSelect,
                        MraprecTypeMap);

/*
Used for unit testing only.

*/

static Operator unittest1("unittest1",
                          unittestspec,
                          Unittest1ValueMap,
                          simpleSelect,
                          Unittest1TypeMap);
static Operator unittest2("unittest2",
                          unittestspec,
                          Unittest2ValueMap,
                          simpleSelect,
                          Unittest2TypeMap);

/*
1 Algebra creation

*/

class MovingRegionAlgebra : public Algebra {
public:
    MovingRegionAlgebra() : Algebra() {
        AddTypeConstructor(&iregion);
        AddTypeConstructor(&uregion);
        AddTypeConstructor(&mregion);

        iregion.AssociateKind("TEMPORAL");
        iregion.AssociateKind("DATA");

        uregion.AssociateKind("TEMPORAL");
        uregion.AssociateKind("DATA");

        mregion.AssociateKind("TEMPORAL");
        mregion.AssociateKind("DATA");

        AddOperator(&atinstant);
        AddOperator(&initial);
        AddOperator(&final);
        AddOperator(&inst);
        AddOperator(&val);
        AddOperator(&deftime);
        AddOperator(&present);
        AddOperator(&intersection);
        AddOperator(&inside);
        AddOperator(&traversed);
        AddOperator(&at);
        AddOperator(&mraprec);

/*
Used for unit testing only.

*/
        AddOperator(&unittest1);
        AddOperator(&unittest2);
    }
    ~MovingRegionAlgebra() {}
};

static MovingRegionAlgebra movingRegionAlgebra;

extern "C"
Algebra* InitializeMovingRegionAlgebra(NestedList* nlRef,
                                       QueryProcessor *qpRef) {
    nl = nlRef;
    qp = qpRef;
    return &movingRegionAlgebra;
}
