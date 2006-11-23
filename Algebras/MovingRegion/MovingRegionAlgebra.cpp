/*

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]
//[ue] [\"u]
//[ae] [\"a]
//[oe] [\"o]

\setcounter{tocdepth}{4}

[1] The ~MovingRegionAlgebra~

December 2005, initial version created by Holger M[ue]nx for bachelor
thesis with Prof. Dr. G[ue]ting, Fachbereich Informatik,
Fernuniversit[ae]t Hagen.

December 2005, Victor Almeida deleted the deprecated algebra levels
(~executable~, ~descriptive~, and ~hibrid~). Only the executable
level remains. Models are also removed from type constructors.

January-June 2006, various bugfixes and improvements by Holger M[ue]nx,
with the help of Victor Almeida and Thomas Behr.

November 2006, Christian D[ue]ntgen added operator ~units~. 
To this end, an additional creator for URegion and assignment operators 
have been intoduced.

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

The class definitions of ~MSegmentData~, 
~TrapeziumSegmentIntersection~, ~RefinementPartitionOrig~,
~URegion~ and ~MRegion~, which
are implemented in ~MovingRegionAlgebra.cpp~, have been moved to
the header file ~MovingRegionAlgebra.h~ to facilitate development work on 
top of the ~MovingRegionAlgebra~ without modifying the ~MovingRegionAlgebra~ 
itself. This file contains detailed descriptions for the usage of the methods
of these classes too.

The MovingRegionAlgebra has been developed for the bachelor thesis of
Holger M[ue]nx together with Prof. Dr. G[ue]ting, Fachbereich Informatik,
Fernuniversit[ae]t Hagen.

1 Known bugs, issues and missing features

Open:

  * Bug: List representation checks incorrect for
    ~(update mv := ((movingregion)((0.0 0.0 true true)(0.0 0.0 1.0 1.0))));~.
    Aleksej Struk found this issue.

    Update: Debug output at the beginning on ~InMRegion()~ indicated that
    this problem actually occurs before ~InMRegion()~ is called. The problem
    is not caused by the ~MovingRegionAlgebra~.

  * Feature: Calculations with values of datatype $Instant$ are done with
    double precision only and not with the datatypes own calculation
    operations.

  * Feature: Operator ~traversed~ is not yet implemented. All code which
    has been prepared for this operator so far is enclosed with 
    ~\#ifdef MRA\_TRAVERSED ... \#endif // MRA\_TRAVERSED~.

  * Feature: Debug bug output is very verbose. Due to its verbosity, it
    has impact on the algebra's performance, when enabled. It would be useful
    to have different debug levels.

Closed:

  * Bug: Lets consider two units ~up~ of type ~upoint~ and ~ur~ of type
    ~uregion~ with the same interval. If ~val(initial(up))~ is within
    ~val(initial(ur))~ and ~val(initial(up))~ is not within 
    ~val(final(ur))~, the operators ~intersection~ and ~inside~ correctly
    recognize the intersection with the edge of the region but do not
    correctly calculate the intervals when the point is within and outside 
    the region. If ~val(initial(up))~ is within ~val(final(ur))~, everything
    works. Please note that this bug is causing two failed unit tests.

    Resolved: Simplifying ~URegion::RestrictedIntersectionFindNormal()~
    fixed the issue.

  * Bug: The ~URegion~ constructor, which constructs the instance from a 
    ~Region~ instance, does not work properly if the region contains half
    segments, where the two end points have been swapped during half segment
    created. J[ue]rgen Schmidt spotted this problem.

    Resolved: Half segments with swapped end points are now being correctly
    handled.

  * Bug: There is a number of assertions, which can be triggered by input
    data. This is caused by rounding errors, which render assumptions 
    verified before inapplicable later.

    Resolved: Failed asserts replaced by proper error handling. Additionally,
    some method stubs have been replaced by proper code. If the remaining
    asserts fail, this will be caused by bugs in the algebra logic or by
    calls to non-implemented functions or methods, where stubs had to be
    provided.

  * Bug: J[oe]rg Schmidt thinks there is an issue in the
    generation of the refinement partition if two intervals start or
    end at the same instant.

    Resolved: Bugfix implemented, storing end instant of previously
    created interval was incorrect.

  * Bug: Sorting units at the beginning of RefinementPartitionOrig() is missing.
    Constructor only works if units appear in ~mr~ and ~mp~ in proper order!

    Rejected: ~InMRegion()~ and ~InMapping()~ sort units by using the
    ~StartBulkLoad()~ and ~EndBulkLoad()~ methods (the latter with parameter
    ~true~, of course).

  * Bug: ~initial~ and ~final~ resulting in failed ~assert()~ when unit's
    interval is open in the respective instant.

    Resolved: Added the required checks.

  * Bug: Objects created in the server version of SECONDO are not
    compatible with the stand-alone version of SECONDO (and vice versa).

    Resolved: Replaced weird implementation of ~OpenMRegion()~ and
    ~SaveMRegion()~ with code based on functions ~Open()~ and ~Save()~ 
    from ~Attribute~. Thomas Behr contributed this valuable hint!

  * Bug: ~URegion::AddSegment()~ checks for moving segment intersections
    outside current region unit, which is incorrect.

    Resolved: Offset ~segmentsStartPos~ in ~URegion::AddSegment()~ was
    missing.

  * Bug: Awkward memory leak at the end of ~URegion::TemporalFunction()~.

    Resolved: Freeing memory now. Resulting code may not be compatible
    with other compilers than gcc.

  * Bug: References to ~URegion~ and ~UPoint~ instead of ~Unit1~ and ~Unit2~
    in ~RefinementPartitionOrig()~. Victor Almeida found this one!

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

1 Unit testing

To facilitate automated unit testing, a large number of test cases is provided
together with a ~Makefile~ to execute them. These are located in the directory
~Tests~. See the file ~Tests/README~ on running the test cases.

Special SECONDO operators have been implemented to make internal 
functionality of the ~MovingRegionAlgebra~ available for unit testing.
These operators are included in the list of test cases below. 

Please note that you have to compile this algebra with the ~MRA\_UNITTEST~
symbol defined so that the specific unit testing operators are available.
Even though this is rendering unit testing slightly more complicated,
this approach has been chosen to avoid puzzling users with operators, which
are not required for the production version of this algebra. The best place
to define this symbol is the ~makefile~, where the proper preparations have
been already done at the head of the file.

The following groups of test cases are available:

  * ~Tests/trapeziumintersection.tests~: Checks the functions to calculate
    the intersection of two trapeziums. These functions are required to
    check the segments of moving regions for validity. The special operator
    ~unittest1~ allows unit testing to directly access these functions.

  * ~Tests/insideabove.tests~: Verifies the ~insideAbove~ attribute of
    segments in moving regions. The special operator ~unittest2~ provides
    direct access to the value of this attribute for a specific segment.

  * ~Tests/refinementpartition.tests~: Assures that the calculation of
    refinement partitions is correct. The special operator ~unittest3~ 
    provides the refinement partition of two ~mpoint~ values.

  * ~Tests/basic.tests~: Checks operators ~bbox~, ~inst~, ~val~, ~deftime~ 
    and present.

  * ~Tests/atinstant.tests~: Checks operators ~atinstant~, ~initial~ and
    ~final~.

  * ~Tests/intersection.tests~: Checks operators ~intersection~, ~inside~ 
    and ~at~.

  * ~Tests/relation.tests~: Checks moving regions as tuple attributes of
    relations.

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
#include "MovingRegionAlgebra.h"
#include "DateTime.h"

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

The operator ~mraprec~ can be used to set ~eps~, ie. to adjust the 
precision of double
comparisons. Please note that the symbol ~MRA\_PREC~ has to be defined to
include this operator in the algebra. The best place to do this is the
~makefile~, and this has been already prepared at the top of the file.

A special behaviour has been implemented for the check whether a moving
segment is collinear with itself in its initial and ifnal instant (see
section \ref{collinear}. Since this check is very prone to rounding errors,
the additional constant ~epsRelaxFactor~ has been introduced. During this
check, two floating point numbers are considered equal if their difference
is less than the constant $eps\cdot\ epsRelaxFactor$. ~epsRelaxFactor~
can be set with the operator ~mraprec~ too.

*/

//const double eps = 0.0001;
//const double eps = 0.00000001;
double eps = 0.00001;
double epsRelaxFactor = 10;

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

1.1 Function ~GausTransform()~

Apply the Gauss transformation to system of equations. The left hand sides
of the equations are in matrix $a$ with $n$ rows and $m$ columns and the
right hand sides are in vector $b$ with $n$ rows.

The transformed matrix and vector are returned in $a$ and $b$.

*/
static void GaussTransform(const unsigned int n,
                           const unsigned int m,
                           double** a,
                           double* b) {
    if (MRA_DEBUG) {
        cerr << "GaussTransform() called, n="
             << n << " m=" << m << endl;

        for (unsigned int j = 0; j < n; j++) {
            for (unsigned int k = 0; k < m; k++)
                fprintf(stderr, "%7.3f ", a[j][k]);
            fprintf(stderr, "| %7.3f\n", b[j]);
        }
        cerr << "============================="
             << "============================="
             << endl;
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

        if (MRA_DEBUG) {
            for (j = 0; j < n; j++) {
                for (unsigned int k = 0; k < m; k++)
                    fprintf(stderr, "%7.3f ", a[j][k]);
                fprintf(stderr, "| %7.3f\n", b[j]);
            }
            cerr 
                << "-----------------------------"
                << "-----------------------------"
                << endl;
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
        if (MRA_DEBUG) cerr << "#1a" << endl;

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
        if (MRA_DEBUG) cerr << "#1b" << endl;

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
        if (MRA_DEBUG) cerr << "#2a" << endl;

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
        if (MRA_DEBUG) cerr << "#2b" << endl;

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

    if (MRA_DEBUG) {
        cerr << "T1B      T1A                "
             << "T2B      T2A" 
             << endl;
        cerr << "-------  ---------------    "
             << "-------  ---------------" 
             << endl;
        for (unsigned int i = 0; i < 3; i++)
            fprintf(stderr, 
                    "%7.3f  %7.3f %7.3f    %7.3f  %7.3f %7.3f\n",
                    T1B[i],
                    T1A[i][0],
                    T1A[i][1],
                    T2B[i],
                    T2A[i][0],
                    T2A[i][1]);
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

                if (MRA_DEBUG) {
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
                }

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

        if (MRA_DEBUG)
            cerr << "specialTrapeziumIntersects() c1=0 f="
                 << f
                 << endl;

        for (unsigned int i = 0; i < 3; i++) {
            P[i] = T2B[i]+T2A[i][1]*f;
            Q[i] = T2A[i][0];
        }
    } else if (nearlyEqual(Ap[2][3], 0.0)) {
/*
Case 2: $c2=0$.

*/

        double f = B[2]/Ap[2][2]; // = b/c1

        if (MRA_DEBUG)
            cerr << "specialTrapeziumIntersects() c2=0 f="
                 << f
                 << endl;

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

            detailedResult = 10;
            return false;
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
            if (MRA_DEBUG) cerr << "#3a" << endl;
            t1zMin = t1zMin > 0 ? 0 : t1zMin;
            t1zMax = t1zMax < 0 ? 0 : t1zMax;
            t1Intersects = true;
        }
        if (specialSegmentIntersects2(
                dt, t1p3x, t1p3y, t1p4x, t1p4y, P, Q)) {
            if (MRA_DEBUG) cerr << "#3b" << endl;
            t1zMin = t1zMin > dt ? dt : t1zMin;
            t1zMax = t1zMax < dt ? dt : t1zMax;
            t1Intersects = true;
        }
        if (specialSegmentIntersects1(
                dt,
                t1p1x, t1p1y, t1p3x, t1p3y,
                P[0], P[1], P[0]+Q[0], P[1]+Q[1])) {
            if (MRA_DEBUG) cerr << "#3c" << endl;
            t1zMin = t1zMin > z ? z : t1zMin;
            t1zMax = t1zMax < z ? z : t1zMax;
            t1Intersects = true;
        }
        if (specialSegmentIntersects1(
                dt,
                t1p2x, t1p2y, t1p4x, t1p4y,
                P[0], P[1], P[0]+Q[0], P[1]+Q[1],
                z)) {
            if (MRA_DEBUG) cerr << "#3d" << endl;
            t1zMin = t1zMin > z ? z : t1zMin;
            t1zMax = t1zMax < z ? z : t1zMax;
            t1Intersects = true;
        }

        double t2zMin = dt;
        double t2zMax = 0;

        bool t2Intersects = false;

        if (specialSegmentIntersects2(
                0, t2p1x, t2p1y, t2p2x, t2p2y, P, Q)) {
            if (MRA_DEBUG) cerr << "#4a" << endl;
            t2zMin = t2zMin > 0 ? 0 : t2zMin;
            t2zMax = t2zMax < 0 ? 0 : t2zMax;
            t2Intersects = true;
        }
        if (specialSegmentIntersects2(
                dt, t2p3x, t2p3y, t2p4x, t2p4y, P, Q)) {
            if (MRA_DEBUG) cerr << "#4b" << endl;
            t2zMin = t2zMin > dt ? dt : t2zMin;
            t2zMax = t2zMax < dt ? dt : t2zMax;
            t2Intersects = true;
        }
        if (specialSegmentIntersects1(
                dt,
                t2p1x, t2p1y, t2p3x, t2p3y,
                P[0], P[1], P[0]+Q[0], P[1]+Q[1],
                z)) {
            if (MRA_DEBUG) cerr << "#4c" << endl;
            t2zMin = t2zMin > z ? z : t2zMin;
            t2zMax = t2zMax < z ? z : t2zMax;
            t2Intersects = true;
        }
        if (specialSegmentIntersects1(
                dt,
                t2p2x, t2p2y, t2p4x, t2p4y,
                P[0], P[1], P[0]+Q[0], P[1]+Q[1],
                z)) {
            if (MRA_DEBUG) cerr << "#4d" << endl;
            t2zMin = t2zMin > z ? z : t2zMin;
            t2zMax = t2zMax < z ? z : t2zMax;
            t2Intersects = true;
        }

        if (MRA_DEBUG) {
            cerr << "specialTrapeziumIntersects() dt="
                 << dt
                 << endl;
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

                    // assert(l1p1is || l1p2is);
                    if (!l1p1is && !l1p2is) {
                        stringstream msg;
                        msg << "Contradicting results for intersection" 
                            << endl
                            << " between trapezium and segment (#1)."
                            << endl
                            << "Trapezium is: ("
                            << l1p1x << " " << l1p1y
                            << ") ("
                            << l1p2x << " " << l1p2y
                            << ") ("
                            << l2p1x << " " << l2p1y
                            << ") ("
                            << l2p2x << " " << l2p2y
                            << ")"
                            << endl
                            << "Segment is: ("
                            << p1x << " " << p1y
                            << ") ("
                            << p2x << " " << p2y
                            << ")"
                            << endl
                            << "It is very likely that this has been"
                            << " caused by rounding errors."
                            << endl
                            << "Try adjusting the precision with the "
                            << " mraprec operator (if available in this"
                            << " build of the MovingRegionAlgebra)."
                            << endl;
                        throw invalid_argument(msg.str());
                    }

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

                    // assert(l1p1is || l1p2is);
                    if (!l1p1is && !l1p2is) {
                        stringstream msg;
                        msg << "Contradicting results for intersection" 
                            << endl
                            << " between trapezium and segment (#2)."
                            << endl
                            << "Trapezium is: ("
                            << l1p1x << " " << l1p1y
                            << ") ("
                            << l1p2x << " " << l1p2y
                            << ") ("
                            << l2p1x << " " << l2p1y
                            << ") ("
                            << l2p2x << " " << l2p2y
                            << ")"
                            << endl
                            << "Segment is: ("
                            << p1x << " " << p1y
                            << ") ("
                            << p2x << " " << p2y
                            << ")"
                            << endl
                            << "It is very likely that this has been"
                            << " caused by rounding errors."
                            << endl
                            << "Try adjusting the precision with the "
                            << " mraprec operator (if available in this"
                            << " build of the MovingRegionAlgebra)."
                            << endl;
                        throw invalid_argument(msg.str());
                    }

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

                        // assert(!l1p1is && !l1p2is); 
                        if (l1p1is || l1p2is) {
                            stringstream msg;
                            msg << "Contradicting results for intersection" 
                                << endl
                                << " between trapezium and segment (#2)."
                                << endl
                                << "Trapezium is: ("
                                << l1p1x << " " << l1p1y
                                << ") ("
                                << l1p2x << " " << l1p2y
                                << ") ("
                                << l2p1x << " " << l2p1y
                                << ") ("
                                << l2p2x << " " << l2p2y
                                << ")"
                                << endl
                                << "Segment is: ("
                                << p1x << " " << p1y
                                << ") ("
                                << p2x << " " << p2y
                                << ")"
                                << endl
                                << "It is very likely that this has been"
                                << " caused by rounding errors."
                                << endl
                                << "Try adjusting the precision with the "
                                << " mraprec operator (if available in this"
                                << " build of the MovingRegionAlgebra)."
                                << endl;
                            throw invalid_argument(msg.str());
                        }

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

Returns ~true~ if point $(x, y)$ is located left or above or on segment 
spanned by the points $(p1x, p1y)$ and $(p2x, p2y)$. This matches the
definition of the ~insideAbove~ attribute in ~Region~ and ~URegion~
instances.

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
1 Supporting classes and class template

Supporting classes are those, which are not registered as SECONDO datatypes
but are used to implement the SECONDO datatypes ~intimeregion~, ~uregion~ and
~movingregion~.

1.1 Class ~MSegmentData~

This class is used to represent the segments, which are used to represent
region units in section \ref{uregion}.

1.1.1 Class definition

The class definition has been moved to ~MovingRegionAlgebra.h~.

1.1.1 Constructor
\label{collinear}

*/

MSegmentData::MSegmentData(
    unsigned int fno,
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
             << faceno << " " << cycleno << " " << segmentno
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
        
        collinear = 
            abs((iey-isy)/(iex-isx)-(fey-fsy)/(fex-fsx)) 
              <= eps*epsRelaxFactor
            || abs((iey-isy)*(fex-fsx)-(fey-fsy)*(iex-isx)) 
                 <= eps*epsRelaxFactor;
        
        if (!collinear) {
            cerr << setprecision(10)
                 << "parameters for segment orientation comparison:"
                 << endl
                 << "  1. (iey-isy)/(iex-isx) = " 
                 << (iey-isy)/(iex-isx) 
                 << endl
                 << "  2. (fey-fsy)/(fex-fsx) = " 
                 << (fey-fsy)/(fex-fsx)
                 << endl
                 << "  3. (iey-isy)*(fex-fsx) = " 
                 << (iey-isy)*(fex-fsx) 
                 << endl
                 << "  4. (fey-fsy)*(iex-isx) = " 
                 << (fey-fsy)*(iex-isx)
                 << endl
                 << "1. and 2. or 3. and 4. should be equal."
                 << endl;
        }

        if (MRA_DEBUG) {
            cerr << ::std::fixed << ::std::setprecision(6);
            cerr << "MSegmentData::MSegmentData() isx=" << isx
                 << " isy=" << isy
                 << " iex=" << iex
                 << " iey=" << iey
                 << " fsx=" << fsx
                 << " fsy=" << fsy
                 << " fex=" << fex
                 << " fey=" << fey
                 << endl;
            cerr << "MSegmentData::MSegmentData() (iey-isy)/(iex-isx)=" 
                 << (iey-isy)/(iex-isx) << endl;
            cerr << "MSegmentData::MSegmentData() (fey-fsy)/(fex-fsx)=" 
                 << (fey-fsy)/(fex-fsx) << endl;
            cerr << "MSegmentData::MSegmentData() (iey-isy)*(fex-fsx)=" 
                 << (iey-isy)*(fex-fsx) << endl;
            cerr << "MSegmentData::MSegmentData() (fey-fsy)*(iex-isx)=" 
                 << (fey-fsy)*(iex-isx) << endl;
            cerr << "MSegmentData::MSegmentData() collinear=" 
                 << collinear << endl;
        }
    }

    if (!collinear)
        throw
            invalid_argument(
                "initial and final segment not collinear");
}

/*
1.1.1 Other methods

1.1.1.1 Method ~restrictToInterval()~

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
1.1.1.1 Method ~ToString()~

*/

string MSegmentData::ToString(void) const {
    ostringstream tmp;

    tmp << "f/c/s=" << faceno 
        << "/" << cycleno 
        << "/" << segmentno
        << " initial: p/d/dn=" << pointInitial 
        << "/" << degeneratedInitial 
        << "/" << degeneratedInitialNext
        << " (" << initialStartX
        << ", " << initialStartY
        << ")-(" << initialEndX
        << ", " << initialEndY
        << ") final: p/d/dn=" << pointFinal
        << "/" << degeneratedFinal 
        << "/" << degeneratedFinalNext
        << " (" << finalStartX
        << ", " << finalStartY
        << ")-(" << finalEndX
        << ", " << finalEndY
        << ")";

    return tmp.str();
}

/*
1.1 Class ~TrapeziumSegmentIntersection~

1.1.1 Class definition

The class definition has been moved to ~MovingRegionAlgebra.h~.

1.1.1 Operator ~<~
 
*/

bool TrapeziumSegmentIntersection::operator<(
    const TrapeziumSegmentIntersection& tsi) const {

    if (nearlyEqual(t, tsi.t)) {
        return type < tsi.type;
    } else
        return t < tsi.t;
}

/*
1.1 Class template ~RefinementPartitionOrig~

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

The class template is called ~RefinementPartitionOrig~ and not
~RefinementPartition~ because J[ue]rgen Schmidt decided to implement
his own version of ~RefinementPartition~ in his ~TemporalLiftedAlgebra~,
resulting in conflicts. The postfix ~Orig~ indicates who was here first!
:-)

1.1.1 Class template definition

The class template definition has been moved to ~MovingRegionAlgebra.h~.

1.1.1 Constructor and destructor

*/

template<class Mapping1, class Mapping2, class Unit1, class Unit2>
RefinementPartitionOrig<Mapping1, Mapping2, Unit1, Unit2>
::RefinementPartitionOrig(
    Mapping1& mr,
    Mapping2& mp) {
    if (MRA_DEBUG)
        cerr << "RP::RP() called" << endl;

    assert(mr.IsOrdered());
    assert(mp.IsOrdered());

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
    const Unit1 *ur;
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
    bool first = true;

    if (ur->timeInterval.start < up->timeInterval.start) {
        t = ur->timeInterval.start;
        c = !ur->timeInterval.lc;
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
                 << ur->timeInterval.start.ToDouble()
                 << " "
                 << ur->timeInterval.end.ToDouble()
                 << " "
                 << ur->timeInterval.lc
                 << " "
                 << ur->timeInterval.rc
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

        if (ur->timeInterval.start.Compare(
                &up->timeInterval.start) == 0
            && ur->timeInterval.end.Compare(
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
            if (!(ur->timeInterval.lc && up->timeInterval.lc)) {
                if (ur->timeInterval.lc)
                    AddUnits(
                        mrUnit,
                        -1,
                        ur->timeInterval.start,
                        ur->timeInterval.start,
                        true,
                        true);
                else if (up->timeInterval.lc)
                    AddUnits(
                        -1,
                        mpUnit,
                        ur->timeInterval.start,
                        ur->timeInterval.start,
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
                ur->timeInterval.start,
                ur->timeInterval.end,
                ur->timeInterval.lc && up->timeInterval.lc,
                ur->timeInterval.rc && up->timeInterval.rc);

/*
If one of the intervals is open and one is closed on the right side,
we have to add a point interval to the refinement partition. This determines
the value of ~c~ too, which we use to remember the open/closed flag of the
last interval added to the refinement partition.

*/
            if (!(ur->timeInterval.rc && up->timeInterval.rc)) {
                if (ur->timeInterval.rc) {
                    AddUnits(
                        mrUnit,
                        -1,
                        ur->timeInterval.end,
                        ur->timeInterval.end,
                        true,
                        true);
                    c = true;
                } else if (up->timeInterval.rc) {
                    AddUnits(
                        -1,
                        mpUnit,
                        ur->timeInterval.end,
                        ur->timeInterval.end,
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
            t = ur->timeInterval.end;

/*
Since we added an interval for both unit interval, increase both unit
pointers.

*/
            if (++mrUnit < mr.GetNoComponents())
                mr.Get(mrUnit, ur);
            if (++mpUnit < mp.GetNoComponents())
                mp.Get(mpUnit, up);
        } else if (ur->timeInterval.Inside(up->timeInterval)) {
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
            if (t < ur->timeInterval.start)
                AddUnits(
                    -1,
                    mpUnit,
                    t > up->timeInterval.start
                    ? t
                    : up->timeInterval.start,
                    ur->timeInterval.start,
                    !c,
                    !ur->timeInterval.lc);

/*
Now add interval from ~mr~. Since it is entirely embedded into the interval
from ~mp~, we directly use its border open/close flags.

*/
            AddUnits(
                mrUnit,
                mpUnit,
                ur->timeInterval.start,
                ur->timeInterval.end,
                ur->timeInterval.lc,
                ur->timeInterval.rc);

/*
Remember right border, right border flag and increase unit pointer ~mrUnit~.

*/
            t = ur->timeInterval.end;
            c = ur->timeInterval.rc;

            if (++mrUnit < mr.GetNoComponents())
                mr.Get(mrUnit, ur);
        } else if (up->timeInterval.Inside(ur->timeInterval)) {
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
                    t > ur->timeInterval.start
                      ? t
                      : ur->timeInterval.start,
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
        } else if (ur->timeInterval.Intersects(up->timeInterval)) {
/*
\textbf{Case 4:} The intervals intersect. Further analysis is required,
which will be handled by sub-cases of case 4.

*/
            if (MRA_DEBUG)
                cerr << "RP::RP()   intersect" << endl;

            if (ur->timeInterval.start.Compare(
                    &up->timeInterval.end) == 0
                && ur->timeInterval.lc
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
                    ur->timeInterval.start,
                    ur->timeInterval.start,
                    true,
                    true);

/*
Remember point interval border and increase unit pointer ~mpUnit~.

*/
                t = ur->timeInterval.start;
                c = true;

                if (++mpUnit < mp.GetNoComponents())
                    mp.Get(mpUnit, up);
            } else if (ur->timeInterval.end.Compare(
                           &up->timeInterval.start) == 0
                       && ur->timeInterval.rc
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

                if (t < ur->timeInterval.end)
                    AddUnits(
                        mrUnit,
                        -1,
                        t > ur->timeInterval.start
                          ? t
                          : ur->timeInterval.start,
                        ur->timeInterval.end,
                        !c,
                        false);

                AddUnits(
                    mrUnit,
                    mpUnit,
                    up->timeInterval.start,
                    up->timeInterval.start,
                    true,
                    true);

                t = ur->timeInterval.end;
                c = true;

                if (++mrUnit < mr.GetNoComponents())
                    mr.Get(mrUnit, ur);
            } else if (ur->timeInterval.start.Compare(
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
                        t > ur->timeInterval.start
                          ? t
                          : ur->timeInterval.start,
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
                    ur->timeInterval.end,
                    up->timeInterval.lc,
                    ur->timeInterval.rc);

/*
Remember interval border and increase unit pointer ~mrUnit~.

*/
                t = ur->timeInterval.end;
                c = ur->timeInterval.rc;

                if (++mrUnit < mr.GetNoComponents())
                    mr.Get(mrUnit, ur);
            } else if (ur->timeInterval.start.Compare(
                           &up->timeInterval.start) == 0) {
/*
\textbf{Case 4.4:} Both intervals have the same start point.

The following assertion holds or we would have cases 2 or 3, which have been
already checked.

*/
                assert(!ur->timeInterval.lc || !up->timeInterval.lc);

/*
Case 4.4 is broken into two sub-cases again.

*/
                if (ur->timeInterval.end.Compare(
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
                    if (first || t < ur->timeInterval.start || !c)
                        AddUnits(
                            mrUnit,
                            -1,
                            ur->timeInterval.start,
                            ur->timeInterval.start,
                            true,
                            true);

/*
Add intersection to refinement partition.

*/

                    AddUnits(
                        mrUnit,
                        mpUnit,
                        ur->timeInterval.start,
                        ur->timeInterval.end,
                        false,
                        ur->timeInterval.rc);

/*
Remember interval border and increase unit pointer ~mrUnit~.

*/
                    t = ur->timeInterval.end;
                    c = ur->timeInterval.rc;

                    if (++mrUnit < mr.GetNoComponents())
                        mr.Get(mrUnit, ur);
                } else {
/*
\textbf{Case 4.4.2:} The right border of the interval from ~mr~ is greater
than the right border of the interval from ~mp~.

The following assertion holds or we would have cases 2 or 3, which have been
already checked.

*/
                    assert(ur->timeInterval.end.Compare(
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
                    if (t < up->timeInterval.start || !c)
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
                        ur->timeInterval.start,
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

                if (t < ur->timeInterval.start)
                    AddUnits(
                        -1,
                        mpUnit,
                        t > up->timeInterval.start
                          ? t
                          : up->timeInterval.start,
                        ur->timeInterval.start,
                        !c,
                        !ur->timeInterval.lc);

                AddUnits(
                    mrUnit,
                    mpUnit,
                    ur->timeInterval.start,
                    up->timeInterval.end,
                    ur->timeInterval.lc,
                    up->timeInterval.rc);

                t = up->timeInterval.end;
                c = up->timeInterval.rc;

                if (++mpUnit < mp.GetNoComponents())
                    mp.Get(mpUnit, up);
            }
        } else if (ur->timeInterval.end.Compare(
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
                ur->timeInterval.end,
                !c,
                ur->timeInterval.rc);

            t = up->timeInterval.start;
            c = !up->timeInterval.lc;

            if (++mrUnit < mr.GetNoComponents()) {
                mr.Get(mrUnit, ur);

/*
Check if the next interval of ~mr~ starts between the previous interval
and the current interval in ~mp~. If so, note this in ~t~ and ~c~.

No special handling of ~c~ is required if both intervals start in the
same instance since ~c~ is not used in these cases (see cases 1, 2, 3,
4.4.1 and 4.4.2 above).

*/
                if (ur->timeInterval.start < t) {
                    t = ur->timeInterval.start;
                    c = !ur->timeInterval.lc;
                }
            }
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
                up->timeInterval.rc);

            t = ur->timeInterval.start;
            c = !ur->timeInterval.lc;

            if (++mpUnit < mp.GetNoComponents()) {
                mp.Get(mpUnit, up);

/*
Check if the next interval of ~mp~ starts between the previous interval
and the current interval in ~mr~. If so, note this in ~t~ and ~c~.

No special handling of ~c~ is required if both intervals start in the
same instance since ~c~ is not used in these cases (see cases 1, 2, 3,
4.4.1 and 4.4.2 above).

*/
                if (up->timeInterval.start < t) {
                    t = up->timeInterval.start;
                    c = !up->timeInterval.lc;
                }
            }
        }

        first = false;
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
        if (t < ur->timeInterval.end)
            AddUnits(
                mrUnit,
                -1,
                t,
                ur->timeInterval.end,
                !c,
                ur->timeInterval.rc);
        mrUnit++;

/*
Now just add the remaining unit intervals to the refinement partition.

*/
        while (mrUnit < mr.GetNoComponents()) {
            mr.Get(mrUnit, ur);

            AddUnits(
                mrUnit,
                -1,
                ur->timeInterval.start,
                ur->timeInterval.end,
                ur->timeInterval.lc,
                ur->timeInterval.rc);

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

template<class Mapping1, class Mapping2, class Unit1, class Unit2>
RefinementPartitionOrig<Mapping1, Mapping2, Unit1, Unit2>
::~RefinementPartitionOrig() {
    if (MRA_DEBUG)
        cerr << "RP::~RP() called" << endl;

    for (unsigned int i = 0; i < iv.size(); i++) delete iv[i];
}

/*
1.1.1 Methods

1.1.1.1 Method template ~Size()~

*/
template<class Mapping1, class Mapping2, class Unit1, class Unit2>
unsigned int RefinementPartitionOrig<Mapping1, Mapping2, Unit1, Unit2>
::Size(void) {
    if (MRA_DEBUG)
        cerr << "RP::Size() called" << endl;
    
    return iv.size();
}

/*
1.1.1.1 Method template ~Get()~

*/
template<class Mapping1, class Mapping2, class Unit1, class Unit2>
void RefinementPartitionOrig<Mapping1, Mapping2, Unit1, Unit2>
::Get(unsigned int pos,
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

/*
1.1.1.1 Method template ~AddUnits()~

*/
template<class Mapping1, class Mapping2, class Unit1, class Unit2>
void RefinementPartitionOrig<Mapping1, Mapping2, Unit1, Unit2>
::AddUnits(
    const int urPos,
    const int upPos,
    const Instant& start,
    const Instant& end,
    const bool lc,
    const bool rc) {

    if (MRA_DEBUG) {
        cerr << "RP::AddUnits() called start="
             << start.ToString()
             << " end="
             << end.ToString()
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

1.1 Class ~IRegion~

1.1.1 Class definition

The class definition has been moved to ~MovingRegionAlgebra.h~.

1.1.1 Constructors

*/
IRegion::IRegion(bool dummy) {
    if (MRA_DEBUG) cerr << "IRegion::IRegion() #2 called" << endl;

/*
This is quite ugly and may not work with other compilers than gcc.
Since the ~Intime<Alpha>()~ constructors do not properly initialise their
~value~ attribute (which if of type ~Region~ in this case), there is
no better solution right now to assure that ~value~ has a valid DBArray.

*/
    Region* tmp = new Region(0);
    memcpy(&value, tmp, sizeof(*tmp));
    free(tmp);
}

IRegion::IRegion(const IRegion& ir) {
    if (MRA_DEBUG) cerr << "IRegion::IRegion() #2 called" << endl;

    instant = ir.instant;
    defined = ir.defined;
    
/*
This is quite ugly and may not work with other compilers than gcc.
Since the ~Intime<Alpha>()~ constructors do not properly initialise their
~value~ attribute (which if of type ~Region~ in this case), there is
no better solution right now to assure that ~value~ has a valid DBArray.

*/
    Region* tmp = new Region(0);
    memcpy(&value, tmp, sizeof(*tmp));
    free(tmp);
    if (ir.defined) value.CopyFrom(&ir.value);
}

/*
1.1.1 Methods for algebra integration

1.1.1.1 Method ~Clone()~

*/
IRegion* IRegion::Clone(void) const {
    if (MRA_DEBUG) cerr << "IRegion::Clone() called" << endl;

    return new IRegion(*this);
}

/*
1.1.1.1 ~DBArray~ access

*/
int IRegion::NumOfFLOBs(void) const {
    if (MRA_DEBUG) cerr << "IRegion::NumOfFLOBs() called" << endl;

    return 1;
}

FLOB* IRegion::GetFLOB(const int i) {
    if (MRA_DEBUG) cerr << "IRegion::GetFLOB() called" << endl;

    assert(i == 0);
    return value.GetFLOB(0);
}

/*
1.1 Algebra integration

1.1.1 Function ~IRegionProperty()~

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

/*
1.1.1 Function ~CheckIRegion()~

*/
static bool CheckIRegion(ListExpr type, ListExpr& errorInfo) {
    if (MRA_DEBUG) cerr << "CheckIRegion() called" << endl;

    return nl->IsEqual(type, "intimeregion");
}

/*
1.1.1 Function ~CreateIRegion()~

*/
static Word CreateIRegion(const ListExpr typeInfo) {
    if (MRA_DEBUG) cerr << "CreateIRegion() called" << endl;

    return SetWord(new IRegion(false));
}

/*
1.1.1 Type constructor ~iregion~

*/
static TypeConstructor iregion(
    "intimeregion",
    IRegionProperty,
    OutIntime<Region, OutRegion>,
    InIntime<Region, InRegion>,
    0, 0, // SaveToList, RestoreFromList
    CreateIRegion,
    DeleteIntime<Region>,
    0, 0, // open, save
    CloseIntime<Region>,
    CloneIntime<Region>,
    CastIntime<Region>,
    SizeOfIntime<Region>,
    CheckIRegion );

/*
1 Class ~URegionEmb~

1.1 Class definition

The class definition has been moved to ~MovingRegionAlgebra.h~.

1.1 Constructors

*/

URegionEmb::URegionEmb(const Interval<Instant>& tiv,
                       unsigned int pos) :
    segmentsStartPos(pos),
    segmentsNum(0),
    bbox(false),
    timeInterval(tiv) {

    if (MRA_DEBUG)
        cerr << "URegionEmb::URegionEmb() #2 called"
             << endl;
}



URegionEmb::URegionEmb(
    DBArray<MSegmentData>* segments,
    const Interval<Instant>& tiv,
    const Region& region,
    unsigned int pos) :
    segmentsStartPos(pos),
    segmentsNum(0),
    timeInterval(tiv) {

    if (MRA_DEBUG)
        cerr << "URegionEmb::URegionEmb() #4 called" << endl;
    
/*
The following code adds each segment of the region as constant moving segment
to the region unit. Since region units do not use half segments, only one 
half segment of each segment is considered. Each considered half segment may
need to be reversed before it is added to the region unit so that clockwise
or counter-clockwise order is maintained within the region unit.

*/

    int cycleStart = 0;

    for (int i = 0; i < region.Size(); i += 2) {
        const HalfSegment *thisHs;
        region.Get(i, thisHs);

        const HalfSegment *nextHs;
        region.Get(i+2 == region.Size() ? 0 : i+2, nextHs);

        if (thisHs->GetAttr().cycleno != nextHs->GetAttr().cycleno) {
            region.Get(cycleStart, nextHs);
            cycleStart = i+2;
        }

        if (MRA_DEBUG) {
            cerr << "URegionEmb::URegionEmb() i=" << i << endl; 
            cerr << "URegionEmb::URegionEmb() cycleStart=" 
                 << cycleStart 
                 << endl; 
            cerr << "URegionEmb::URegionEmb() thisHs=" << *thisHs << endl;
            cerr << "URegionEmb::URegionEmb() nextHs=" << *nextHs << endl;
        }

        if (thisHs->GetRightPoint() == nextHs->GetLeftPoint()
            || thisHs->GetRightPoint() == nextHs->GetRightPoint()) {
            if (MRA_DEBUG) 
                cerr << "URegionEmb::URegionEmb() not swapping" << endl; 

            MSegmentData dms(thisHs->GetAttr().faceno,
                             thisHs->GetAttr().cycleno,
                             thisHs->GetAttr().edgeno,
                             thisHs->GetAttr().insideAbove,
                             thisHs->GetLeftPoint().GetX(),
                             thisHs->GetLeftPoint().GetY(),
                             thisHs->GetRightPoint().GetX(),
                             thisHs->GetRightPoint().GetY(),
                             thisHs->GetLeftPoint().GetX(),
                             thisHs->GetLeftPoint().GetY(),
                             thisHs->GetRightPoint().GetX(),
                             thisHs->GetRightPoint().GetY());

            dms.SetDegeneratedInitial(DGM_NONE);
            dms.SetDegeneratedFinal(DGM_NONE);

            if (MRA_DEBUG)
                cerr << "URegionEmb::URegionEmb() adding " 
                     << dms.ToString() 
                     << endl;

            segments->Put(segmentsStartPos+segmentsNum, dms);
        } else {
            if (MRA_DEBUG) 
                cerr << "URegionEmb::URegionEmb() swapping" << endl; 

            MSegmentData dms(thisHs->GetAttr().faceno,
                             thisHs->GetAttr().cycleno,
                             thisHs->GetAttr().edgeno,
                             thisHs->GetAttr().insideAbove,
                             thisHs->GetRightPoint().GetX(),
                             thisHs->GetRightPoint().GetY(),
                             thisHs->GetLeftPoint().GetX(),
                             thisHs->GetLeftPoint().GetY(),
                             thisHs->GetRightPoint().GetX(),
                             thisHs->GetRightPoint().GetY(),
                             thisHs->GetLeftPoint().GetX(),
                             thisHs->GetLeftPoint().GetY());

            dms.SetDegeneratedInitial(DGM_NONE);
            dms.SetDegeneratedFinal(DGM_NONE);
            
            if (MRA_DEBUG)
                cerr << "URegionEmb::URegionEmb() adding " 
                     << dms.ToString() 
                     << endl;

            segments->Put(segmentsStartPos+segmentsNum, dms);
        }

        segmentsNum++;
    }

    Rectangle<2> rbb = region.BoundingBox();
    double min[3] = { rbb.MinD(0), 
                      rbb.MinD(1), 
                      timeInterval.start.ToDouble() };
    double max[3] = { rbb.MaxD(0), 
                      rbb.MaxD(1), 
                      timeInterval.end.ToDouble() };
    bbox.Set(true, min, max);
}


void URegionEmb::SetSegmentsNum(int i){
  segmentsNum=i;
}

void URegionEmb::SetStartPos(int i){
  segmentsStartPos=i;
}

/*
1.1 Methods required to act as unit in ~Mapping~ template

1.1.1 Method ~IsValid()~

*/
bool URegionEmb::IsValid(void) const {
    return true;
}

/*

1.1 Method ~Sizeof()~

*/
size_t URegion::Sizeof() const {
    if (MRA_DEBUG) cerr << "URegion::Sizeof() called" << endl;

    return sizeof(*this);
}

/*
1.1.1 Method ~Disjoint()~

*/
bool URegionEmb::Disjoint(const URegionEmb& ur) const {
    return 
        timeInterval.R_Disjoint(ur.timeInterval)
        || ur.timeInterval.R_Disjoint(timeInterval);
}

/*
1.1.1 Method ~TU\_Adjacent()~

*/
bool URegionEmb::TU_Adjacent(const URegionEmb& ur) const {
    return
        timeInterval.R_Adjacent(ur.timeInterval)
        || ur.timeInterval.R_Adjacent(timeInterval);
}

/*
1.1.1 Operator ~==~

*/
bool URegionEmb::operator==(const URegionEmb& ur) const {
    return timeInterval == ur.timeInterval;
}

/*
1.1.1 Method ~Before()~

*/
bool URegionEmb::Before(const URegionEmb& ur) const {
    return timeInterval.Before(ur.timeInterval);
}

/*
1.1.1 Method ~Compare()~

*/
bool URegionEmb::Compare(const URegionEmb* ur) const {
    assert(false);
    return false;
}



/*
1.1 Moving segment access methods

1.1.1 Method ~GetSegmentsNum()~

*/
int URegionEmb::GetSegmentsNum(void) const {
    if (MRA_DEBUG)
        cerr << "URegionEmb::GetSegmentsNum() called, num="
             << segmentsNum
             << endl;

    return segmentsNum;
}


/*
1.1.2 Method ~GetStartPos()~

*/
const int URegionEmb::GetStartPos() const{
    if (MRA_DEBUG)
        cerr << "URegionEmb::GetStartPos() called, sp="
             << segmentsStartPos
             << endl;

    return segmentsStartPos;
}


/*
1.1.1 Method ~GetSegment()~

*/
void URegionEmb::GetSegment(
    const DBArray<MSegmentData>* segments,
    int pos, 
    const MSegmentData*& dms) const {

    if (MRA_DEBUG)
        cerr << "URegionEmb::GetSegment() called, pos=" << pos
             << ", segments="
             << segments
             << endl;

    segments->Get(segmentsStartPos+pos, dms);
}

/*
1.1.1 Method ~PutSegment()~

*/
void URegionEmb::PutSegment(
    DBArray<MSegmentData>* segments,
    int pos, 
    const MSegmentData& dms,
    const bool isNew) {

    if (MRA_DEBUG)
        cerr << "URegionEmb::PutSegment() called, pos=" << pos
             << endl;

    segments->Put(segmentsStartPos+pos, dms);
    if(isNew){
        segmentsNum++;
    }
}

/*
1.1.1 Method ~SetSegmentInsideAbove()~

*/
void URegionEmb::SetSegmentInsideAbove(
    DBArray<MSegmentData>* segments,
    int pos, 
    bool insideAbove) {

    if (MRA_DEBUG)
        cerr << "URegionEmb::GetSegmentLeftOrAbove() called, segments=" 
             << segments
             << endl;

    const MSegmentData *auxDms;
    segments->Get(segmentsStartPos+pos, auxDms);
    MSegmentData dms( *auxDms );
    dms.SetInsideAbove(insideAbove);
    segments->Put(segmentsStartPos+pos, dms);
}

/*
1.1.1 Method ~AddSegment()~

*/
bool URegionEmb::AddSegment(
    DBArray<MSegmentData>* segments,
    Region& cr,
    Region& rDir,
    unsigned int faceno,
    unsigned int cycleno,
    unsigned int segmentno,
    unsigned int partnerno,
    double intervalLen,
    ListExpr start,
    ListExpr end) {

    if (MRA_DEBUG)
        cerr << "URegionEmb::AddSegment() called "
             << faceno
             << " "
             << cycleno
             << " "
             << segmentno
             << endl;

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
                "  Start point "
                +nl->ToString(start)
                +" not in format (<number> <number> <number> <number>).");

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
                "  End point "
                +nl->ToString(end)
                +" not in format (<number> <number> <number> <number>).");

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
            cerr << "URegionEmb::AddSegment() segmentsNum="
                 << segmentsNum
                 << endl;

/*
For each of the already existing segments:

*/
        for (int i = segmentsNum-1; i >= 0; i--) {
            if (MRA_DEBUG)
                cerr << "URegionEmb::AddSegment() i=" << i << endl;

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
            if (dms.GetDegeneratedInitialNext() < 0
                && !dms.GetPointInitial()
                && !existingDms.GetPointInitial()
                && ((nearlyEqual(
                         dms.GetInitialStartX(),
                         existingDms.GetInitialStartX())
                     && nearlyEqual(
                            dms.GetInitialStartY(),
                            existingDms.GetInitialStartY())
                     && nearlyEqual(
                            dms.GetInitialEndX(),
                            existingDms.GetInitialEndX())
                     && nearlyEqual(
                            dms.GetInitialEndY(),
                            existingDms.GetInitialEndY()))
                    || (nearlyEqual(
                            dms.GetInitialStartX(),
                            existingDms.GetInitialEndX())
                        && nearlyEqual(
                                dms.GetInitialStartY(),
                                existingDms.GetInitialEndY())
                        && nearlyEqual(
                                dms.GetInitialEndX(),
                                existingDms.GetInitialStartX())
                        && nearlyEqual(
                                dms.GetInitialEndY(),
                                existingDms.GetInitialStartY())))) {
                if (MRA_DEBUG)
                    cerr << "URegionEmb::AddSegment() "
                         << "degen'ed initial in " << i
                         << endl;

                dms.SetDegeneratedInitialNext(0);
                existingDms.SetDegeneratedInitialNext(segmentsNum+1);

                segments->Put(segmentsStartPos+i, existingDms);
            }

/*
Same for the final instant.

*/
            if (dms.GetDegeneratedFinalNext() < 0
                && !dms.GetPointFinal()
                && !existingDms.GetPointFinal()
                && ((nearlyEqual(
                         dms.GetFinalStartX(),
                         existingDms.GetFinalStartX())
                     && nearlyEqual(
                            dms.GetFinalStartY(),
                            existingDms.GetFinalStartY())
                     && nearlyEqual(
                            dms.GetFinalEndX(),
                            existingDms.GetFinalEndX())
                     && nearlyEqual(
                            dms.GetFinalEndY(),
                            existingDms.GetFinalEndY()))
                    || (nearlyEqual(
                            dms.GetFinalStartX(),
                            existingDms.GetFinalEndX())
                        && nearlyEqual(
                               dms.GetFinalStartY(),
                               existingDms.GetFinalEndY())
                        && nearlyEqual(
                               dms.GetFinalEndX(),
                               existingDms.GetFinalStartX())
                        && nearlyEqual(
                               dms.GetFinalEndY(),
                               existingDms.GetFinalStartY())))) {
                if (MRA_DEBUG)
                    cerr << "URegionEmb::AddSegment() "
                         << "degen'ed final in " << i
                         << endl;

                dms.SetDegeneratedFinalNext(0);
                existingDms.SetDegeneratedFinalNext(segmentsNum+1);

                segments->Put(segmentsStartPos+i, existingDms);
            }

/*
If we have a point time interval, degeneration is not allowed.

*/
            if (nearlyEqual(intervalLen, 0.0)
                && (dms.GetDegeneratedInitialNext() >= 0
                    || dms.GetDegeneratedFinalNext() >= 0)) {
                stringstream msg;
                msg << "  Units with zero length time interval must not"
                    << " be degenerated." << endl
                    << "    New segment:" << endl
                    << "      Initial: ("
                    << dms.GetInitialStartX()
                    << " "
                    << dms.GetInitialStartY()
                    << ") - ("
                    << dms.GetInitialEndX()
                    << " "
                    << dms.GetInitialEndY()
                    << ")" << endl
                    << "      Final: ("
                    << dms.GetFinalStartX()
                    << " "
                    << dms.GetFinalStartY()
                    << ") - ("
                    << dms.GetFinalEndX()
                    << " "
                    << dms.GetFinalEndY();
                throw invalid_argument(msg.str());
            } else if (!nearlyEqual(intervalLen, 0.0)
                     && dms.GetDegeneratedInitialNext() >= 0
                     && dms.GetDegeneratedFinalNext() >= 0) {
                stringstream msg;
                msg << "  Units must not degenerate both in initial and"
                    << " final instant." << endl
                    << "    New segment:" << endl
                    << "      Initial: ("
                    << dms.GetInitialStartX()
                    << " "
                    << dms.GetInitialStartY()
                    << ") - ("
                    << dms.GetInitialEndX()
                    << " "
                    << dms.GetInitialEndY()
                    << ")" << endl
                    << "      Final: ("
                    << dms.GetFinalStartX()
                    << " "
                    << dms.GetFinalStartY()
                    << ") - ("
                    << dms.GetFinalEndX()
                    << " "
                    << dms.GetFinalEndY();
                throw invalid_argument(msg.str());
            }

/*
Check if the current segment intersects with the existing segment.
Since both a moving segments and are spanning a trapezium in 3d space
$(x, y, t)$, this is equivalent to the intersection of two trapeziums.

*/
            unsigned int detailedResult;

            if (specialTrapeziumIntersects(intervalLen,
                                           existingDms.GetInitialStartX(),
                                           existingDms.GetInitialStartY(),
                                           existingDms.GetInitialEndX(),
                                           existingDms.GetInitialEndY(),
                                           existingDms.GetFinalStartX(),
                                           existingDms.GetFinalStartY(),
                                           existingDms.GetFinalEndX(),
                                           existingDms.GetFinalEndY(),
                                           dms.GetInitialStartX(),
                                           dms.GetInitialStartY(),
                                           dms.GetInitialEndX(),
                                           dms.GetInitialEndY(),
                                           dms.GetFinalStartX(),
                                           dms.GetFinalStartY(),
                                           dms.GetFinalEndX(),
                                           dms.GetFinalEndY(),
                                           detailedResult)) {

                stringstream msg;
                msg << "  Moving segments intersect (code "
                    << detailedResult
                    << ")." << endl
                    << "    Existing segment:" << endl
                    << "      Initial: ("
                    << existingDms.GetInitialStartX()
                    << " "
                    << existingDms.GetInitialStartY()
                    << ") - ("
                    << existingDms.GetInitialEndX()
                    << " "
                    << existingDms.GetInitialEndY()
                    << ")" << endl
                    << "      Final:  ("
                    << existingDms.GetFinalStartX()
                    << " "
                    << existingDms.GetFinalStartY()
                    << ") - ("
                    << existingDms.GetFinalEndX()
                    << " "
                    << existingDms.GetFinalEndY()
                    << ")" << endl
                    << "    New segment:" << endl
                    << "      Initial: ("
                    << dms.GetInitialStartX()
                    << " "
                    << dms.GetInitialStartY()
                    << ") - ("
                    << dms.GetInitialEndX()
                    << " "
                    << dms.GetInitialEndY()
                    << ")" << endl
                    << "      Final:  ("
                    << dms.GetFinalStartX()
                    << " "
                    << dms.GetFinalStartY()
                    << ") - ("
                    << dms.GetFinalEndX()
                    << " "
                    << dms.GetFinalEndY();
                throw invalid_argument(msg.str());
            }
        }

/*
Add half segments to $cr$ and $rDir$ for region check and direction
computation.

*/
        double t = nearlyEqual(intervalLen, 0.0) ? 0 : 0.5;
        double xs =
            dms.GetInitialStartX()
            +(dms.GetFinalStartX()-dms.GetInitialStartX())*t;
        double ys =
            dms.GetInitialStartY()
            +(dms.GetFinalStartY()-dms.GetInitialStartY())*t;
        double xe =
            dms.GetInitialEndX()
            +(dms.GetFinalEndX()-dms.GetInitialEndX())*t;
        double ye =
            dms.GetInitialEndY()
            +(dms.GetFinalEndY()-dms.GetInitialEndY())*t;

        Point s(true, xs, ys);
        Point e(true, xe, ye);
        HalfSegment hs(true, s, e);

        hs.attr.faceno = faceno;
        hs.attr.cycleno = cycleno;
        hs.attr.partnerno = partnerno;
        hs.attr.edgeno = segmentno;

        hs.attr.insideAbove = hs.GetLeftPoint() == s;

        if (MRA_DEBUG)
            cerr << "URegionEmb::AddSegment() "
                 << dms.ToString()
                 << endl;

        if (MRA_DEBUG)
            cerr << "URegionEmb::AddSegment() "
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
                 << hs.attr.insideAbove
                 << endl;

        if (!cr.InsertOk(hs)) {
            stringstream msg;
            msg << "  Region checks for segment failed." << endl
                << "    New segment:" << endl
                << "      Initial: ("
                << dms.GetInitialStartX()
                << " "
                << dms.GetInitialStartY()
                << ") - ("
                << dms.GetInitialEndX()
                << " "
                << dms.GetInitialEndY()
                << ")" << endl
                << "      Final:  ("
                << dms.GetFinalStartX()
                << " "
                << dms.GetFinalStartY()
                << ") - ("
                << dms.GetFinalEndX()
                << " "
                << dms.GetFinalEndY();
            throw invalid_argument(msg.str());
        }

        cr += hs;
        if( hs.IsLeftDomPoint() )
        {
          rDir += hs;
          hs.SetLeftDomPoint( false );
        }
        else
        {
          hs.SetLeftDomPoint( true );
          rDir += hs;
        }
        cr += hs;

        segments->Resize(segmentsStartPos+segmentsNum+1);
        segments->Put(segmentsStartPos+segmentsNum, dms);
        segmentsNum++;

        if (bbox.IsDefined()) {
            double min[3] = { bbox.MinD(0), bbox.MinD(1), bbox.MinD(2) };
            double max[3] = { bbox.MaxD(0), bbox.MaxD(1), bbox.MaxD(2) };
            if (dms.GetInitialStartX() < min[0])
                min[0] = dms.GetInitialStartX();
            if (dms.GetFinalStartX() < min[0])
                min[0] = dms.GetFinalStartX();
            if (dms.GetInitialStartY() < min[1])
                min[1] = dms.GetInitialStartY();
            if (dms.GetFinalStartY() < min[1])
                min[1] = dms.GetFinalStartY();
            if (dms.GetInitialStartX() > max[0])
                max[0] = dms.GetInitialStartX();
            if (dms.GetFinalStartX() > max[0])
                max[0] = dms.GetFinalStartX();
            if (dms.GetInitialStartY() > max[1])
                max[1] = dms.GetInitialStartY();
            if (dms.GetFinalStartY() > max[1])
                max[1] = dms.GetFinalStartY();
            bbox.Set(true, min, max);

        } else {
            double min[3] =
                { dms.GetInitialStartX() < dms.GetFinalStartX()
                  ? dms.GetInitialStartX() : dms.GetFinalStartX(),
                  dms.GetInitialStartY() < dms.GetFinalStartY()
                  ? dms.GetInitialStartY() : dms.GetFinalStartY(),
                  timeInterval.start.ToDouble() };
            double max[3] =
                { dms.GetInitialStartX() > dms.GetFinalStartX()
                  ? dms.GetInitialStartX() : dms.GetFinalStartX(),
                  dms.GetInitialStartY() > dms.GetFinalStartY()
                  ? dms.GetInitialStartY() : dms.GetFinalStartY(),
                  timeInterval.end.ToDouble() };
            bbox.Set(true, min, max);
        }
    } catch (invalid_argument& e) {
        cerr << "-----------------------------------------------------------"
             << endl
             << "Checking segment failed."
             << endl
             << e.what()
             << endl
             << "-----------------------------------------------------------"
             << endl;
        return false;
    }

    return true;
}

/*
1.1 Helper methods for ~RestrictedIntersection()~

1.1.1 Method ~Plumbline()~

*/
unsigned int URegionEmb::Plumbline(
    const DBArray<MSegmentData>* segments,
    const UPoint& up, 
    const Interval<Instant>& iv) const{

    if (MRA_DEBUG) cerr << "URegionEmb::Plumbline() called" << endl;

/*
Since we know that ~up~ is not intersecting with any of the segments, we
can pick any time in ~iv~ to examine ~up~ and the segments. We pick the
middle of the interval:

*/
    double t =
        iv.start.ToDouble()
        +(iv.end.ToDouble()-iv.start.ToDouble())/2;

    if (MRA_DEBUG)
        cerr << "URegionEmb::Plumbline() iv.start="
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
        cerr << "URegionEmb::Plumbline() f=" << f << endl;

    if (MRA_DEBUG)
        cerr << "URegionEmb::Plumbline() p0=("
             << up.p0.GetX() << " " << up.p0.GetY()
             << ") p1=("
             << up.p1.GetX() << " " << up.p1.GetY()
             << ")"
             << endl;

    double x = up.p0.GetX()+(up.p1.GetX()-up.p0.GetX())*f;
    double y = up.p0.GetY()+(up.p1.GetY()-up.p0.GetY())*f;

    if (MRA_DEBUG)
        cerr << "URegionEmb::Plumbline() x=" << x << " y=" << y << endl;

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
            cerr << "URegionEmb::Plumbline() segment #" << i << endl;

        const MSegmentData *dms;
        segments->Get(segmentsStartPos+i, dms);

/*
Calculate position of segment at ~t~.

*/
        double p1x =
            dms->GetInitialStartX()
            +(dms->GetFinalStartX()-dms->GetInitialStartX())*f;
        double p1y =
            dms->GetInitialStartY()
            +(dms->GetFinalStartY()-dms->GetInitialStartY())*f;
        double p2x =
            dms->GetInitialEndX()
            +(dms->GetFinalEndX()-dms->GetInitialEndX())*f;
        double p2y =
            dms->GetInitialEndY()
            +(dms->GetFinalEndY()-dms->GetInitialEndY())*f;

        if (MRA_DEBUG)
            cerr << "URegionEmb::Plumbline() p1x=" << p1x
                 << " p1y=" << p1y
                 << " p2x=" << p2x
                 << " p2y=" << p2y
                 << endl;

        if (nearlyEqual(p1x, p2x)) {
/*
Ignore vertical segments.

*/
            if (MRA_DEBUG)
                cerr << "URegionEmb::Plumbline() "
                     << "vertical, ignored"
                     << endl;
        } else if (nearlyEqual(p1x, x) && lowerOrNearlyEqual(y, p1y)) {
/*
~(x, y)~ is one of the endpoints. Only count it if it is the right endpoint.

*/
            if (MRA_DEBUG)
                cerr << "URegionEmb::Plumbline() "
                     << "plumbline through start point"
                     << endl;

            if (p1x > p2x) {
                if (MRA_DEBUG)
                    cerr << "URegionEmb::Plumbline() counted" << endl;

                num++;
            } else {
                if (MRA_DEBUG)
                    cerr << "URegionEmb::Plumbline() ignored" << endl;
            }
        } else if (nearlyEqual(p2x, x) && lowerOrNearlyEqual(y, p2y)) {
/*
~(x, y)~ is one of the endpoints. Only count it if it is the right endpoint.

*/
            if (MRA_DEBUG)
                cerr << "URegionEmb::Plumbline() "
                     << "plumbline through end point"
                     << endl;

            if (p1x < p2x) {
                if (MRA_DEBUG)
                    cerr << "URegionEmb::Plumbline() counted" << endl;

                num++;
            } else {
                if (MRA_DEBUG)
                    cerr << "URegionEmb::Plumbline() ignored" << endl;
            }
        } else if (!between(p1x, x, p2x)) {
/*
~(x, y)~ is not below the segment.

*/
            if (MRA_DEBUG)
                cerr << "URegionEmb::Plumbline() "
                     << "x not between p1x, p1y, ignored"
                     << endl;
        } else {
/*
~(x, y)~ is in the same ~x~-range as the segment. Count it if the segment
is above ~(x, y)~.

*/
            if (MRA_DEBUG)
                cerr << "URegionEmb::Plumbline() "
                     << "checking for intersection"
                     << endl;

            if (MRA_DEBUG) {
                cerr << "URegionEmb::Plumbline() nearlyEqual(p1x, x)="
                     << nearlyEqual(p1x, x)
                     << endl;
                cerr << "URegionEmb::Plumbline() nearlyEqual(p2x, x)="
                     << nearlyEqual(p2x, x)
                     << endl;
            }

            // solve p1x+(p2x-p1x)*s = x

            double s = (x-p1x)/(p2x-p1x);
            double ys = p1y+(p2y-p1y)*s;

            if (MRA_DEBUG)
                cerr << "URegionEmb::Plumbline() ys=" << ys << endl;

            if (lowerOrNearlyEqual(y, ys)) {
                if (MRA_DEBUG)
                    cerr << "URegionEmb::Plumbline() below" << endl;

                num++;
            } else {
                if (MRA_DEBUG)
                    cerr << "URegionEmb::Plumbline() not below" << endl;
            }
        }

        if (MRA_DEBUG)
            cerr << "URegionEmb::Plumbline() num=" << num << endl;
    }

    return num;
}

/*
1.1.1 Method ~RestrictedIntersectionAddUPoint()~

*/
void URegionEmb::RestrictedIntersectionAddUPoint(MPoint& res,
                                                 double starttime,
                                                 double endtime,
                                                 bool lc,
                                                 bool rc,
                                                 double x0,
                                                 double y0,
                                                 double x1,
                                                 double y1,
                                                 UPoint*& pending,
                                                 bool merge) const {
    if (MRA_DEBUG) {
        cerr << "URegionEmb::RIAUP() called, merge=" << merge << endl;
        cerr << "URegionEmb::RIAUP() starttime=" 
             << setprecision(12)
             << starttime
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

    if (MRA_DEBUG)
        cerr << "URegionEmb::RIAUP() as instant start="
             << start.ToString()
             << " end="
             << end.ToString()
             << endl;

/*
Straightforward again. If there is a previous unit pending, try to merge
it. If we cannot merge it, put it into ~res~ and start new pending
unit.

(I am not sure, though, whether merging pending units is required. It may
reduce the number of units produced - but is this really a requirement?)

*/
    if (pending) {
        if (MRA_DEBUG)
            cerr << "URegionEmb::RIAUP() pending exists" << endl;

        if (MRA_DEBUG) {
            cerr << "URegionEmb::RIAUP() pending: ["
                 << setprecision(8)
                 << pending->timeInterval.start.ToString()
                 << " "
                 << pending->timeInterval.end.ToString()
                 << " "
                 << pending->timeInterval.lc
                 << " "
                 << pending->timeInterval.rc
                 << "] ("
                 << pending->p0.GetX() << " " << pending->p0.GetY()
                 << ")-("
                 << pending->p1.GetX() << " " << pending->p1.GetY()
                 << ")"
                 << endl;
            cerr << "URegionEmb::RIAUP() current: ["
                 << setprecision(8)
                 << start.ToString()
                 << " "
                 << end.ToString()
                 << " "
                 << lc
                 << " "
                 << rc
                 << "] ("
                 << x0 << " " << y0 
                 << ")-(" 
                 << x1 << " " << y1 
                 << ")"
                 << endl;
        }

        if (merge
            && nearlyEqual(pending->timeInterval.end.ToDouble(),
                           starttime)
            && (pending->timeInterval.rc || lc)
            && nearlyEqual(pending->p1.GetX(), x0)
            && nearlyEqual(pending->p1.GetY(), y0)) {

            if (MRA_DEBUG)
                cerr << "URegionEmb::RIAUP() intervals match" << endl;

            if (nearlyEqual(pending->timeInterval.start.ToDouble(),
                            pending->timeInterval.end.ToDouble())) {
                if (MRA_DEBUG)
                    cerr << "URegionEmb::RIAUP() merge #1" << endl;

                Interval<Instant> iv(start,
                                     end,
                                     true,
                                     rc);

                delete pending;
                pending = new UPoint(iv, x0, y0, x1, y1);

                if (MRA_DEBUG)
                    cerr << "URegionEmb::RIAUP() pending="
                         << (unsigned int) pending
                         << endl;

                return;
            } else if (nearlyEqual(starttime, endtime)) {
                if (MRA_DEBUG)
                    cerr << "URegionEmb::RIAUP() merge #2" << endl;

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
                    cerr << "URegionEmb::RIAUP() pending="
                         << (unsigned int) pending
                         << endl;

                return;
            } else {
                if (MRA_DEBUG)
                    cerr << "URegionEmb::RIAUP() checking endpoints"
                         << endl;

                double f =
                    (pending->timeInterval.start.ToDouble()
                     -starttime)
                    /(endtime-starttime);

                double x = x0+(x1-x0)*f;
                double y = y0+(y1-y0)*f;

                if (MRA_DEBUG)
                    cerr << "URegionEmb::RIAUP() f=" 
                         << f
                         << " -> x="
                         << x
                         << " y="
                         << y
                         << endl;


                if (nearlyEqual(pending->p0.GetX(), x)
                    && nearlyEqual(pending->p0.GetY(), y)) {
                    if (MRA_DEBUG)
                        cerr << "URegionEmb::RIAUP() merge #3" << endl;

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
                        cerr << "URegionEmb::RIAUP() pending="
                             << (unsigned int) pending
                             << endl;

                    return;
                }
            }
        }
    }

    if (MRA_DEBUG) cerr << "URegionEmb::RIAUP() no merge" << endl;

    if (pending) {
        if (MRA_DEBUG)
            cerr << "URegionEmb::RIAUP() adding "
                 << (unsigned int) pending
                 << setprecision(12)
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
        cerr << "URegionEmb::RIAUP() pending="
             << (unsigned int) pending
             << endl;

}

/*
1.1.1 Method ~RestrictedIntersectionFindNormal()~

*/
void URegionEmb::RestrictedIntersectionFindNormal(
    const Interval<Instant>& iv,
    const UPoint& rUp,
    MSegmentData& rDms,
    bool& ip1present,
    double& ip1x,
    double& ip1y,
    double& ip1t,
    vector<TrapeziumSegmentIntersection>& vtsi) const {

    if (MRA_DEBUG)
        cerr << "URegionEmb::RIFN() called" << endl;

/*
Calculate end points of moving segment at time of intersection ~ip1t~.

*/
    double p1x, p1y, p2x, p2y;

    if (nearlyEqual(iv.start.ToDouble(), iv.end.ToDouble())) {
        p1x = rDms.GetInitialStartX();
        p1y = rDms.GetInitialStartY();
        p2x = rDms.GetInitialEndX();
        p2y = rDms.GetInitialEndY();
    } else {
        double f =
            (ip1t-iv.start.ToDouble())
            /(iv.end.ToDouble()-iv.start.ToDouble());

        p1x =
            rDms.GetInitialStartX()
            +(rDms.GetFinalStartX()-rDms.GetInitialStartX())*f;
        p1y =
            rDms.GetInitialStartY()
            +(rDms.GetFinalStartY()-rDms.GetInitialStartY())*f;
        p2x =
            rDms.GetInitialEndX()
            +(rDms.GetFinalEndX()-rDms.GetInitialEndX())*f;
        p2y =
            rDms.GetInitialEndY()
            +(rDms.GetFinalEndY()-rDms.GetInitialEndY())*f;
    }

    if (MRA_DEBUG) {
        cerr << "URegionEmb::RIFN() rUp=("
             << rUp.p0.GetX() << " " << rUp.p0.GetY()
             << ")-("
             << rUp.p1.GetX() << " " << rUp.p1.GetY()
             << ")"
             << endl;
        cerr << "URegionEmb::RIFN() p1=("
             << p1x << " " << p1y
             << ") p2=("
             << p2x << " " << p2y
             << ")"
             << " ip1=("
             << ip1x << " " << ip1y << " " << ip1t << ")"
             << endl;
    }

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
            cerr << "URegionEmb::RIFN() through p0 and p1" << endl;

/*
Yes, lets see if the moving segment did not touch the ~UPoint~ instance
during initial time. If yes, we can deduce from the ~insideAbove~ attribute
and the direction of the movement whether the ~URegion~ has been entered or
left at the intersection point. If not, we just two identical points, one
with TSI\_ENTER and one with TSI\_LEAVE, which indicates that the ~URegion~
has been entered and left in the same instance.

*/
        if (!nearlyEqual(ip1t, iv.start.ToDouble())) {
            if (pointAboveSegment(ip1x, ip1y,
                                  rDms.GetInitialStartX(), 
                                  rDms.GetInitialStartY(),
                                  rDms.GetInitialEndX(), 
                                  rDms.GetInitialEndY())) {
                tsi.type = rDms.GetInsideAbove() ? TSI_LEAVE : TSI_ENTER;
            } else {
                tsi.type = rDms.GetInsideAbove() ? TSI_ENTER : TSI_LEAVE;
            }

            if (MRA_DEBUG)
                cerr << "URegionEmb::RIFN() adding "
                     << tsi.type
                     << " " << tsi.x << " " << tsi.y << " " << tsi.t
                     << endl;
            vtsi.push_back(tsi);
        } else if (!nearlyEqual(ip1t, iv.end.ToDouble())) {
            if (pointAboveSegment(ip1x, ip1y,
                                  rDms.GetFinalStartX(), 
                                  rDms.GetFinalStartY(),
                                  rDms.GetFinalEndX(), 
                                  rDms.GetFinalEndY())) {
                tsi.type = rDms.GetInsideAbove() ? TSI_ENTER : TSI_LEAVE;
            } else {
                tsi.type = rDms.GetInsideAbove() ? TSI_LEAVE : TSI_ENTER;
            }

            if (MRA_DEBUG)
                cerr << "URegionEmb::RIFN() adding "
                     << tsi.type
                     << " " << tsi.x << " " << tsi.y << " " << tsi.t
                     << endl;
            vtsi.push_back(tsi);
        } else {
            if (MRA_DEBUG)
                cerr << "URegionEmb::RIFN() adding "
                     << tsi.type
                     << " " << tsi.x << " " << tsi.y << " " << tsi.t
                     << endl;
            tsi.type = TSI_ENTER;
            vtsi.push_back(tsi);

            if (MRA_DEBUG)
                cerr << "URegionEmb::RIFN() adding "
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
point actually entering the ~URegion~ instance. 

*/
    } else if (nearlyEqual(rUp.p0.GetX(), ip1x)
               && nearlyEqual(rUp.p0.GetY(), ip1y)) {
        if (MRA_DEBUG)
            cerr << "URegionEmb::RIFN() through p0" << endl;

        if (pointAboveSegment(rUp.p1.GetX(), rUp.p1.GetY(),
                              p1x, p1y, p2x, p2y)) {
            tsi.type = rDms.GetInsideAbove() ? TSI_ENTER : TSI_LEAVE;
        } else {
            tsi.type = rDms.GetInsideAbove() ? TSI_LEAVE : TSI_ENTER;
        }

        if (MRA_DEBUG)
            cerr << "URegionEmb::RIFN() adding "
                 << tsi.type
                 << " " << tsi.x << " " << tsi.y << " " << tsi.t
                 << endl;

        vtsi.push_back(tsi);

        if (pointAboveSegment(rUp.p1.GetX(), rUp.p1.GetY(),
                              rDms.GetFinalStartX(), 
                              rDms.GetFinalStartY(),
                              rDms.GetFinalEndX(), 
                              rDms.GetFinalEndY())) {
            tsi.type = tsi.type == TSI_ENTER ? TSI_LEAVE : TSI_ENTER;

            if (MRA_DEBUG)
                cerr << "URegionEmb::RIFN() adding "
                     << tsi.type
                     << " " << tsi.x << " " << tsi.y << " " << tsi.t
                     << endl;

            vtsi.push_back(tsi);
        }

/*
Is the other endpoint of the ~UPoint~ instance located on the segment?
If so, see handling of previous case.

*/

    } else if (nearlyEqual(rUp.p1.GetX(), ip1x)
               && nearlyEqual(rUp.p1.GetY(), ip1y)) {
        if (MRA_DEBUG)
            cerr << "URegionEmb::RIFN() through p1" << endl;

        if (pointAboveSegment(rUp.p0.GetX(), rUp.p0.GetY(),
                              p1x, p1y, p2x, p2y)) {
            tsi.type = rDms.GetInsideAbove() ? TSI_LEAVE : TSI_ENTER;
        } else {
            tsi.type = rDms.GetInsideAbove() ? TSI_ENTER : TSI_LEAVE;
        }

        if (MRA_DEBUG)
            cerr << "URegionEmb::RIFN() adding "
                 << tsi.type
                 << " " << tsi.x << " " << tsi.y << " " << tsi.t
                 << endl;

        vtsi.push_back(tsi);

        if (pointAboveSegment(rUp.p0.GetX(), rUp.p0.GetY(),
                              rDms.GetInitialStartX(), 
                              rDms.GetInitialStartY(),
                              rDms.GetInitialEndX(), 
                              rDms.GetInitialEndY())) {
            tsi.type = tsi.type == TSI_ENTER ? TSI_LEAVE : TSI_ENTER;

            if (MRA_DEBUG)
                cerr << "URegionEmb::RIFN() adding "
                     << tsi.type
                     << " " << tsi.x << " " << tsi.y << " " << tsi.t
                     << endl;

            vtsi.push_back(tsi);
        }

/*
At this point, we do know that one endpoint of the ~UPoint~ instance is
left or above the segment. Lets find out which in the remaining two cases.

*/
    } else if (pointAboveSegment(rUp.p0.GetX(), rUp.p0.GetY(),
                                 rDms.GetInitialStartX(), 
                                 rDms.GetInitialStartY(),
                                 rDms.GetInitialEndX(), 
                                 rDms.GetInitialEndY())) {
        if (MRA_DEBUG)
            cerr << "URegionEmb::RIFN() p0 above segment" << endl;

        tsi.type = rDms.GetInsideAbove() ? TSI_LEAVE : TSI_ENTER;

        if (MRA_DEBUG)
            cerr << "URegionEmb::RIFN() adding "
                 << tsi.type
                 << " " << tsi.x << " " << tsi.y << " " << tsi.t
                 << endl;

        vtsi.push_back(tsi);
    } else {
        if (MRA_DEBUG)
            cerr << "URegionEmb::RIFN() p0 below segment" << endl;

        tsi.type = rDms.GetInsideAbove() ? TSI_ENTER : TSI_LEAVE;

        if (MRA_DEBUG)
            cerr << "URegionEmb::RIFN() adding "
                 << tsi.type
                 << " " << tsi.x << " " << tsi.y << " " << tsi.t
                 << endl;

        vtsi.push_back(tsi);
    }
}


/*
1.1.1 Method ~RestrictedIntersectionFindInPlane()~

*/
void URegionEmb::RestrictedIntersectionFindInPlane(
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
        cerr << "URegionEmb::RIFIP() called" << endl;

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
            cerr << "URegionEmb::RIFIP() up on segment"
                 << endl;

        tsi.type = ip1t < ip2t ? TSI_ENTER : TSI_LEAVE;

        if (MRA_DEBUG)
            cerr << "URegionEmb::RIFIP() adding "
                 << tsi.type
                 << " " << tsi.x << " " << tsi.y << " " << tsi.t
                 << endl;

        vtsi.push_back(tsi);

        tsi.type = ip1t < ip2t ? TSI_LEAVE : TSI_ENTER;
        tsi.x = ip2x;
        tsi.y = ip2y;
        tsi.t = ip2t;

        if (MRA_DEBUG)
            cerr << "URegionEmb::RIFIP() adding "
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
            cerr << "URegionEmb::RIFIP() ip1=p0" << endl;

        tsi.type = TSI_ENTER;

        if (MRA_DEBUG)
            cerr << "URegionEmb::RIFIP() adding "
                 << tsi.type
                 << " " << tsi.x << " " << tsi.y << " " << tsi.t
                 << endl;

        vtsi.push_back(tsi);

        tsi.type = TSI_LEAVE;
        tsi.x = ip2x;
        tsi.y = ip2y;
        tsi.t = ip2t;

        if (MRA_DEBUG)
            cerr << "URegionEmb::RIFIP() adding "
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
            cerr << "URegionEmb::RIFIP() ip1=p1" << endl;

        tsi.type = TSI_LEAVE;

        if (MRA_DEBUG)
            cerr << "URegionEmb::RIFIP() adding "
                 << tsi.type
                 << " " << tsi.x << " " << tsi.y << " " << tsi.t
                 << endl;

        vtsi.push_back(tsi);

        tsi.type = TSI_ENTER;
        tsi.x = ip2x;
        tsi.y = ip2y;
        tsi.t = ip2t;

        if (MRA_DEBUG)
            cerr << "URegionEmb::RIFIP() adding "
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
            cerr << "URegionEmb::RIFIP() ip2=p0" << endl;

        tsi.type = TSI_LEAVE;

        if (MRA_DEBUG)
            cerr << "URegionEmb::RIFIP() adding "
                 << tsi.type
                 << " " << tsi.x << " " << tsi.y << " " << tsi.t
                 << endl;

        vtsi.push_back(tsi);

        tsi.type = TSI_ENTER;
        tsi.x = ip2x;
        tsi.y = ip2y;
        tsi.t = ip2t;

        if (MRA_DEBUG)
            cerr << "URegionEmb::RIFIP() adding "
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
            cerr << "URegionEmb::RIFIP() ip2=p1" << endl;

        tsi.type = TSI_ENTER;

        if (MRA_DEBUG)
            cerr << "URegionEmb::RIFIP() adding "
                 << tsi.type
                 << " " << tsi.x << " " << tsi.y << " " << tsi.t
                 << endl;

        vtsi.push_back(tsi);

        tsi.type = TSI_LEAVE;
        tsi.x = ip2x;
        tsi.y = ip2y;
        tsi.t = ip2t;

        if (MRA_DEBUG)
            cerr << "URegionEmb::RIFIP() adding "
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
            cerr << "URegionEmb::RIFIP() up covers segment" << endl;

        tsi.type = ip1t < ip2t ? TSI_ENTER : TSI_LEAVE;

        if (MRA_DEBUG)
            cerr << "URegionEmb::RIFIP() adding "
                 << tsi.type
                 << " " << tsi.x << " " << tsi.y << " " << tsi.t
                 << endl;

        vtsi.push_back(tsi);

        tsi.type = ip1t < ip2t ? TSI_LEAVE : TSI_ENTER;
        tsi.x = ip2x;
        tsi.y = ip2y;
        tsi.t = ip2t;

        if (MRA_DEBUG)
            cerr << "URegionEmb::RIFIP() adding "
                 << tsi.type
                 << " " << tsi.x << " " << tsi.y << " " << tsi.t
                 << endl;

        vtsi.push_back(tsi);
    }
}

 
/*
1.1.1 Method ~RestrictedIntersectionFind()~

*/

void URegionEmb::RestrictedIntersectionFind(
    const DBArray<MSegmentData>* segments,
    const UPoint& up,
    const Interval<Instant>& iv,
    vector<TrapeziumSegmentIntersection>& vtsi) const {

/*
Straightforward. Even being slightly cryptic, the debug output provides
sufficient context to understand this method.

*/
    if (MRA_DEBUG)
        cerr << "URegionEmb::RIF() called" << endl;

    UPoint rUp(true);
    restrictUPointToInterval(up, iv, rUp);

    for (unsigned int i = 0; i < segmentsNum; i++) {
        const MSegmentData *dms;
        segments->Get(segmentsStartPos+i, dms);

        if (MRA_DEBUG) {
            cerr << "URegionEmb::RIF() segment #"
                 << i
                 << ": ["
                 << timeInterval.start.ToDouble()
                 << " "
                 << timeInterval.end.ToDouble()
                 << " "
                 << timeInterval.lc << " " << timeInterval.rc
                 << "] "
                 << dms->ToString()
                 << endl;
            cerr << "URegionEmb::RIF() point is ["
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
            cerr << "URegionEmb::RIF() iv is ["
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
            cerr << "URegionEmb::RIF() segment restricted to iv is "
                 << rDms.ToString()
                 << endl;
            cerr << "URegionEmb::RIF() point restricted to iv is ("
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
            rDms.GetInitialStartX(), rDms.GetInitialStartY(),
            rDms.GetInitialEndX(), rDms.GetInitialEndY(),
            rDms.GetFinalStartX(), rDms.GetFinalStartY(),
            rDms.GetFinalEndX(), rDms.GetFinalEndY(),
            rUp.p0.GetX(), rUp.p0.GetY(),
            rUp.p1.GetX(), rUp.p1.GetY(),
            ip1present, ip1x, ip1y, ip1t,
            ip2present, ip2x, ip2y, ip2t);

        if (ip1t < iv.start.ToDouble() || ip1t > iv.end.ToDouble())
            ip1present = false;
        if (ip2t < iv.start.ToDouble() || ip2t > iv.end.ToDouble())
            ip2present = false;

        if (MRA_DEBUG) {
            cerr << "URegionEmb::RIF() ip1present=" << ip1present 
                 << endl;
            cerr << "URegionEmb::RIF() ip2present=" << ip2present
                 << endl;
        }

        if (ip1present) {
            if (MRA_DEBUG)
                cerr << "URegionEmb::RIF() intersection" << endl;

            if (ip2present
                && !(nearlyEqual(ip1x, ip2x)
                     && nearlyEqual(ip1y, ip2y)
                     && nearlyEqual(ip1t, ip2t))) {
                if (MRA_DEBUG)
                    cerr << "URegionEmb::RIF() in plane, ip1t=" << ip1t
                         << ", ip2t=" << ip2t
                         << ", interval=" << timeInterval.start.ToDouble()
                         << " " << timeInterval.end.ToDouble()
                         << " " << timeInterval.lc 
                         << " " << timeInterval.rc
                         << ", restricted=" << iv.start.ToDouble()
                         << " " << iv.end.ToDouble()
                         << " " << iv.lc
                         << " " << iv.rc
                         << endl;

                RestrictedIntersectionFindInPlane(
                    iv, rUp,
                    ip1present, ip1x, ip1y, ip1t,
                    ip2present, ip2x, ip2y, ip2t,
                    vtsi);
            } else {
                if (MRA_DEBUG)
                    cerr << "URegionEmb::RIF() not in plane, ip1t=" << ip1t
                         << ", interval=" << timeInterval.start.ToDouble()
                         << " " << timeInterval.end.ToDouble()
                         << " " << timeInterval.lc 
                         << " " << timeInterval.rc
                         << ", restricted=" << iv.start.ToDouble()
                         << " " << iv.end.ToDouble()
                         << " " << iv.lc
                         << " " << iv.rc
                         << endl;

                RestrictedIntersectionFindNormal(
                    iv, rUp, rDms,
                    ip1present, ip1x, ip1y, ip1t,
                    vtsi);
            }
        }
    }
}

/*
1.1.1 Method ~RestrictedIntersectionFix()~

*/
void URegionEmb::RestrictedIntersectionFix(
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
            cerr << "URegionEmb::RIFix() intersection dump #"
                 << i
                 << " type="
                 << vtsi[i].type
                 << " ip=["
                 << vtsi[i].x << " " << vtsi[i].y << " " << vtsi[i].t
                 << "]"
                 << endl;


    for (unsigned int i = 0; i < vtsi.size(); i++) {
        if (MRA_DEBUG)
            cerr << "URegionEmb::RIFix() post-sort i=" << i << endl;

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
            cerr << "URegionEmb::RIFix() post-sort j=" << j << endl;
            cerr << "URegionEmb::RIFix() post-sort numEnter="
                 << numEnter
                 << endl;
            cerr << "URegionEmb::RIFix() post-sort numLeave="
                 << numLeave
                 << endl;
        }

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
1.1.1 Method ~RestrictedIntersectionProcess()~

*/
bool URegionEmb::RestrictedIntersectionProcess(
    const UPoint& up,
    const Interval<Instant>& iv,
    vector<TrapeziumSegmentIntersection>& vtsi,
    MPoint& res,
    UPoint*& pending,
    bool merge) const {

/*
Check all intersections with the same coordinates. Bring them in
useful order. For example, if we are inside region at a specific time
and arrive at a $TSI\_ENTER$ and $TSI\_LEAVE$ intersection, we should
consider the $TSI\_LEAVE$ intersection first.

*/
    if (MRA_DEBUG) 
        cerr << "URegionEmb::RIP() called, interval=" 
             << setprecision(12)
             << iv.start.ToDouble()
             << " "
             << iv.end.ToDouble()
             << " "
             << iv.lc
             << " " 
             << iv.rc
             << endl;

    if (vtsi.size() == 0) return false;

    int prev_i = -1;
    bool prev_c;

    unsigned int pos = 0;

    if (MRA_DEBUG && vtsi.size() > 0)
        cerr << "URegionEmb::RIP() (pre) intersection dump #"
             << 0
             << " type="
             << vtsi[0].type
             << " ip=["
             << vtsi[0].x << " " << vtsi[0].y << " " << vtsi[0].t
             << "]"
             << endl;

    for (unsigned int i = 1; i < vtsi.size(); i++) {
        if (MRA_DEBUG)
            cerr << "URegionEmb::RIP() intersection dump #"
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
                cerr << "URegionEmb::RIP() removing" << endl;
        } else {
            if (MRA_DEBUG)
                cerr << "URegionEmb::RIP() keeping" << endl;

            pos++;
            if (pos < i) vtsi[pos] = vtsi[i];
        }
    }

    if (MRA_DEBUG) 
        for (unsigned int i = 0; i <= pos; i++) 
            cerr << "URegionEmb::RIP() intersection dump #"
                 << setprecision(12)
                 << i
                 << " type="
                 << vtsi[i].type
                 << " ip=["
                 << vtsi[i].x << " " << vtsi[i].y << " " << vtsi[i].t
                 << "]"
                 << endl;

    for (unsigned int i = 0; i <= pos; i++) {
        if (MRA_DEBUG) {
            cerr << "URegionEmb::RIP() intersection dump #"
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
                cerr << "URegionEmb::RIP() special case (start)"
                     << endl;

            bool rc =
                nearlyEqual(vtsi[i].t, iv.end.ToDouble())
                ? iv.rc
                : true;

            UPoint rUp(true);
            restrictUPointToInterval(up, iv, rUp);

            RestrictedIntersectionAddUPoint(
                res,
                iv.start.ToDouble(), vtsi[i].t, iv.lc, rc,
                rUp.p0.GetX(), rUp.p0.GetY(), vtsi[i].x, vtsi[i].y,
                pending,
                merge);

            prev_i = i;
            prev_c = rc;
        }

        if (i > 0 && vtsi[i].type == TSI_LEAVE) {
            if (MRA_DEBUG)
                cerr << "URegionEmb::RIP() normal case" << endl;

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
                pending,
                merge);

            prev_i = i;
            prev_c = rc;
        }

        if (i == pos && vtsi[i].type == TSI_ENTER) {
            if (MRA_DEBUG)
                cerr << "URegionEmb::RIP() special case (end)" << endl;

            bool lc =
                nearlyEqual(vtsi[i].t, iv.start.ToDouble())
                ? iv.lc
                : true;

            if (prev_i >= 0
                && nearlyEqual(vtsi[prev_i].t, vtsi[i].t)
                && prev_c)
                lc = false;

            UPoint rUp(true);
            restrictUPointToInterval(up, iv, rUp);

            RestrictedIntersectionAddUPoint(
                res,
                vtsi[i].t, iv.end.ToDouble(), lc, iv.rc,
                vtsi[i].x, vtsi[i].y, rUp.p1.GetX(), rUp.p1.GetY(),
                pending,
                merge);

            prev_i = 0;
        }
    }

    return prev_i >= 0;
}

/*
1.1 Database operator methods

These methods are used to implement the algebra's operators.

1.1.1 Method ~RestrictedIntersection()~

Checks whether the point unit ~up~ intersects this region unit, while
both units are restricted to the interval ~iv~, which must be inside
the interval of the two units (this is not checked and must be assured
before this method is called!).

*/
void URegionEmb::RestrictedIntersection(const DBArray<MSegmentData>* segments,
                                        const UPoint& up,
                                        const Interval<Instant>& iv,
                                        MPoint& res,
                                        UPoint*& pending,
                                        bool merge) const {
    if (MRA_DEBUG)
        cerr << "URegionEmb::RI() called" << endl;

    vector<TrapeziumSegmentIntersection> vtsi;

    RestrictedIntersectionFind(segments, up, iv, vtsi);

    if (MRA_DEBUG)
        cerr << "URegionEmb::RI() vtsi.size()=" << vtsi.size()
             << endl;

    sort(vtsi.begin(), vtsi.end());

    RestrictedIntersectionFix(vtsi);

    if (!RestrictedIntersectionProcess(up, iv, vtsi, res, pending, merge)) {
        if (MRA_DEBUG)
            cerr << "URegionEmb::RI() no intersection in whole unit"
                 << endl;

        unsigned int num = Plumbline(segments, up, iv);

        if (num > 0 && num % 2 == 1) {
          UPoint rUp(true);
            restrictUPointToInterval(up, iv, rUp);

            RestrictedIntersectionAddUPoint(
                res,
                iv.start.ToDouble(), iv.end.ToDouble(),
                iv.lc, iv.rc,
                rUp.p0.GetX(), rUp.p0.GetY(),
                rUp.p1.GetX(), rUp.p1.GetY(),
                pending,
                merge);
        }
    }
}

/*
1.1.1 Method ~TemporalFunction()~

*/
void URegionEmb::TemporalFunction(
    const DBArray<MSegmentData>* segments,
    const Instant& t, 
    Region& res, 
    bool ignoreLimits ) const {

    if (MRA_DEBUG)
        cerr << "URegionEmb::TemporalFunction() called" << endl;

/*
Straightforward again. Calculate segments at specified instant,
remove degenerated segments of initial or final instant, when they
are not border of any region, and create region.

*/
    assert(t.IsDefined());
    assert(timeInterval.Contains(t) || ignoreLimits);

    Instant t0 = timeInterval.start;
    Instant t1 = timeInterval.end;

    bool initialInstant = nearlyEqual(t0.ToDouble(), t.ToDouble());
    bool finalInstant = nearlyEqual(t1.ToDouble(), t.ToDouble());

    double f =
        nearlyEqual(t0.ToDouble(), t1.ToDouble())
        ? 0
        : (t-t0)/(t1-t0);

    int partnerno = 0;

    res.Clear();

    res.StartBulkLoad();

    for (unsigned int i = 0; i < segmentsNum; i++) {
            if (MRA_DEBUG)
                cerr << "URegionEmb::TemporalFunction() segment #"
                     << i
                     << " ("
                     << segmentsStartPos
                     << ")"
                     << endl;

            const MSegmentData *dms;
            segments->Get(segmentsStartPos+i, dms);

            double xs =
                dms->GetInitialStartX()
                +(dms->GetFinalStartX()-dms->GetInitialStartX())*f;
            double ys =
                dms->GetInitialStartY()
                +(dms->GetFinalStartY()-dms->GetInitialStartY())*f;
            double xe =
                dms->GetInitialEndX()
                +(dms->GetFinalEndX()-dms->GetInitialEndX())*f;
            double ye =
                dms->GetInitialEndY()
                +(dms->GetFinalEndY()-dms->GetInitialEndY())*f;

            if (MRA_DEBUG)
                cerr << "URegionEmb::TemporalFunction()   value is "
                     << xs << " " << ys << " " << xe << " " << ye
                     << endl;

            if (nearlyEqual(xs, xe) && nearlyEqual(ys, ye)) {
                if (MRA_DEBUG)
                    cerr << "URegionEmb::TemporalFunction()   "
                         << "reduced to point"
                         << endl;
                continue;
            }

            assert(dms->GetDegeneratedInitial() != DGM_UNKNOWN);
            assert(dms->GetDegeneratedFinal() != DGM_UNKNOWN);

            if ((initialInstant
                 && dms->GetDegeneratedInitial() == DGM_IGNORE)
                || (finalInstant
                    && dms->GetDegeneratedFinal() == DGM_IGNORE)) {
                if (MRA_DEBUG)
                    cerr << "URegionEmb::TemporalFunction()   "
                         << "ignored degenerated"
                         << endl;
                continue;
            }

            Point s(true, xs, ys);
            Point e(true, xe, ye);
            HalfSegment hs(true, s, e);

            hs.attr.faceno = 0;
            hs.attr.cycleno = 0;
            hs.attr.edgeno = partnerno;
            hs.attr.partnerno = partnerno++;

            if (initialInstant
                 && dms->GetDegeneratedInitial() == DGM_INSIDEABOVE)
                hs.attr.insideAbove = true;
            else if (initialInstant
                 && dms->GetDegeneratedInitial() == DGM_NOTINSIDEABOVE)
                hs.attr.insideAbove = false;
            else if (finalInstant
                 && dms->GetDegeneratedFinal() == DGM_INSIDEABOVE)
                hs.attr.insideAbove = true;
            else if (finalInstant
                 && dms->GetDegeneratedFinal() == DGM_NOTINSIDEABOVE)
                hs.attr.insideAbove = false;
            else
                hs.attr.insideAbove = dms->GetInsideAbove();

            res += hs;
            hs.SetLeftDomPoint(!hs.IsLeftDomPoint());
            res += hs;
    }

    res.EndBulkLoad();

    if (MRA_DEBUG)
        for (int i = 0; i < res.Size(); i++) {
            const HalfSegment *hs;
            res.Get(i, hs);

            cerr << "URegionEmb::TemporalFunction() segment #"
                 << i
                 << " lp=("
                 << hs->GetLeftPoint().GetX()
                 << ", "
                 << hs->GetLeftPoint().GetY()
                 << ") rp=("
                 << hs->GetRightPoint().GetX()
                 << ", "
                 << hs->GetRightPoint().GetY()
                 << ") ldp="
                 << hs->IsLeftDomPoint()
                 << " attr="
                 << hs->attr.faceno
                 << " "
                 << hs->attr.cycleno
                 << " "
                 << hs->attr.edgeno
                 << " "
                 << hs->attr.partnerno
                 << " "
                 << hs->attr.insideAbove
                 << endl;
        }
}

/*
1.1.1 Method ~BoundingBox()~

*/
const Rectangle<3> URegionEmb::BoundingBox() const {
    if (MRA_DEBUG) cerr << "URegionEmb::BoundingBox() called" << endl;

    return bbox;
}

/*
1.1 In and out functions

1.1.1 Function ~InURegionEmbedded()~

*/
static URegionEmb* InURegionEmbedded(
    const ListExpr instance,
    const int errorPos,
    ListExpr& errorInfo,
    DBArray<MSegmentData>* segments,
    unsigned int segmentsStartPos) {

    if (MRA_DEBUG)
        cerr << "InURegionEmbedded() called, segmentsStartPos="
             << segmentsStartPos
             << endl;

/*
Please that ~Region~ creation is done as shown in ~SpatialAlgebra~.
See there for more details.

*/

    if (nl->ListLength(instance) == 0) {
        cerr << "uregion not in format (<interval> <face>*)" << endl;
        return 0;
    }

    if (MRA_DEBUG)
        cerr << "InURegionEmbedded() (<interval> <face>*) found" << endl;

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
        return 0;
    }

    bool correct;

    Instant *start =
        (Instant *) InInstant(nl->TheEmptyList(),
                              nl->First(interval),
                              errorPos,
                              errorInfo,
                              correct).addr;
    if (!correct) {
        cerr << "uregion interval invalid start time" << endl;
        return 0;
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
        return 0;
    }

    bool lc = nl->BoolValue(nl->Third(interval));
    bool rc = nl->BoolValue(nl->Fourth(interval));

    if (end->ToDouble() < start->ToDouble()
        || (nearlyEqual(start->ToDouble(), end->ToDouble())
            && !(lc && rc))) {
        cerr << "uregion invalid interval" << endl;
        delete start;
        delete end;
        return 0;
    }

    Interval<Instant> tinterval(*start, *end, lc, rc);

    double intervalLen = end->ToDouble()-start->ToDouble();

    delete start;
    delete end;

/*
Create ~URegion~ instance and pass storage of segments, if we received
any.

*/
    URegionEmb* uregion = new URegionEmb(tinterval, segmentsStartPos);

    unsigned int faceno = 0;
    unsigned int partnerno = 0;
    ListExpr faces = nl->Second(instance);

    Region cr(0);
    cr.StartBulkLoad();

    if (nl->ListLength(faces) == 0) {
        cerr << "uregion should contain at least one face" << endl;
        delete uregion;
        return 0;
    }

/*
... all this is similar to ~Region~ creation in ~SpatialAlgebra~...

*/
    while (!nl->IsEmpty(faces)) {
        if (MRA_DEBUG)
            cerr << "InURegionEmbedded() face #" << faceno << endl;

        ListExpr cycles = nl->First(faces);

        if (nl->ListLength(cycles) == 0) {
            cerr << "uregion face should contain at least one cycle"
                 << endl;
            delete uregion;
            return 0;
        }

        unsigned int cycleno = 0;
        unsigned int pointno = 0;

        while (!nl->IsEmpty(cycles)) {
            if (MRA_DEBUG)
                cerr << "InURegionEmbedded()   cycle #" << cycleno << endl;

            Region rDir(0);
            rDir.StartBulkLoad();

            ListExpr cyclepoints = nl->First(cycles);

            if (nl->ListLength(cyclepoints) < 3) {
                cerr << "uregion cycle should contain at "
                     << "least three points"
                     << endl;
                delete uregion;
                return 0;
            }

            ListExpr firstPoint = nl->First(cyclepoints);
            ListExpr prevPoint = 0;

            unsigned int initialSegmentsNum =
                uregion->GetSegmentsNum();

            while (!nl->IsEmpty(cyclepoints)) {
                if (MRA_DEBUG)
                    cerr << "InURegionEmbedded()     point #"
                         << pointno
                         << endl;

                ListExpr point = nl->First(cyclepoints);

                if (prevPoint != 0
                    && !uregion->AddSegment(segments,
                                            cr,
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
                    return 0;
                }

                prevPoint = point;
                cyclepoints = nl->Rest(cyclepoints);
                pointno++;
                partnerno++;
            }

            if (!uregion->AddSegment(segments,
                                     cr,
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
                return 0;
            }

            partnerno++;

            rDir.EndBulkLoad(true, true, false, false);

            bool direction = rDir.GetCycleDirection();

            int h = cr.Size()-(rDir.Size()*2);
            int i = initialSegmentsNum;
            while (h < cr.Size()) {
                const HalfSegment *hsInsideAbove;
                bool insideAbove;

                cr.Get(h, hsInsideAbove);

                if (MRA_DEBUG)
                    cerr << "InURegionEmbedded() i="
                         << i
                         << " insideAbove="
                         << hsInsideAbove->attr.insideAbove
                         << endl;

                if (direction == hsInsideAbove->attr.insideAbove)
                    insideAbove = false;
                else
                    insideAbove = true;
                if (cycleno > 0) insideAbove = !insideAbove;


                //hsInsideAbove.attr.insideAbove = insideAbove;
                //cr.UpdateAttr(h, hsInsideAbove.attr);

                //cr.Get(h+1, hsInsideAbove);
                //hsInsideAbove.attr.insideAbove = insideAbove;
                //cr.UpdateAttr(h+1, hsInsideAbove.attr);

                uregion->SetSegmentInsideAbove(segments, i, insideAbove);

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
            uregion->GetSegment(segments, i, dms);

            cerr << "InURegionEmbedded() segment #"
                 << i
                 << ": "
                 << dms->ToString()
                 << endl;
        }

/*
This is different from ~Region~ handling in ~SpatialAlgebra~. We have
to go through the lists of degenerated segments and count how often
a region is inside above in each list. If there are more inside above
segments than others, we need one inside above segment for the
~TemporalFunction()~ and set all others to ignore. Vice version if there
are more inside below segments. If there is the same number of inside above
and inside below segments, we can ignore the entire list.

*/
    for (int i = 0; i < uregion->GetSegmentsNum(); i++) {
        const MSegmentData *auxDms;
        uregion->GetSegment(segments, i, auxDms);
        MSegmentData dms( *auxDms );

        if (!dms.GetPointInitial() && dms.GetDegeneratedInitialNext() < 0)
            nonTrivialInitial = true;
        if (!dms.GetPointFinal() && dms.GetDegeneratedFinalNext() < 0)
            nonTrivialFinal = true;

        if (dms.GetDegeneratedInitial() == DGM_UNKNOWN) {
            if (dms.GetDegeneratedInitialNext() >= 0) {
                const MSegmentData *auxDegenDms;
                MSegmentData degenDms;
                unsigned int numInsideAbove = 0;
                unsigned int numNotInsideAbove = 0;
                for (int j = i+1;
                     j != 0;
                     j = degenDms.GetDegeneratedInitialNext()) {
                    uregion->GetSegment(segments, j-1, auxDegenDms);
                    degenDms = *auxDegenDms;
                    if (MRA_DEBUG)
                        cerr << "InURegionEmbedded() degen-magic-i "
                             << i
                             << " "
                             << j-1
                             << endl;
                    if (degenDms.GetInsideAbove())
                        numInsideAbove++;
                    else
                        numNotInsideAbove++;
                    if (j != i+1) {
                        degenDms.SetDegeneratedInitial(DGM_IGNORE);
                        uregion->PutSegment(segments, j-1, degenDms);
                    }
                }

                if (MRA_DEBUG)
                    cerr << "InURegionEmbedded() degen-magic-i result "
                         << numInsideAbove
                         << " "
                         << numNotInsideAbove
                         << endl;

                if (numInsideAbove == numNotInsideAbove) {
                    dms.SetDegeneratedInitial(DGM_IGNORE);
                } else if (numInsideAbove == numNotInsideAbove+1) {
                    dms.SetDegeneratedInitial(DGM_INSIDEABOVE);
                } else if (numInsideAbove+1 == numNotInsideAbove) {
                    dms.SetDegeneratedInitial(DGM_NOTINSIDEABOVE);
                } else {
                    cerr << "segment ("
                         << dms.GetInitialStartX()
                         << ", " << dms.GetInitialStartY()
                         << ")-(" << dms.GetInitialEndX()
                         << ", " << dms.GetInitialEndY()
                         << ") / (" << dms.GetFinalStartX()
                         << ", " << dms.GetFinalStartY()
                         << ")-(" << dms.GetFinalEndX()
                         << ", " << dms.GetFinalEndY()
                         << ") incorrectly degenerated "
                         << "in initial instant"
                         << endl;

                    delete uregion;
                    return 0;
                }
            } else
                dms.SetDegeneratedInitial(DGM_NONE);
        }

        if (dms.GetDegeneratedFinal() == DGM_UNKNOWN) {
            if (dms.GetDegeneratedFinalNext() >= 0) {
                const MSegmentData *auxDegenDms;
                MSegmentData degenDms;
                unsigned int numInsideAbove = 0;
                unsigned int numNotInsideAbove = 0;
                for (int j = i+1;
                     j != 0;
                     j = degenDms.GetDegeneratedFinalNext()) {
                    uregion->GetSegment(segments, j-1, auxDegenDms);
                    degenDms = *auxDegenDms;
                    if (MRA_DEBUG)
                        cerr << "InURegionEmbedded() degen-magic-f "
                             << i
                             << " "
                             << j-1
                             << endl;
                    if (degenDms.GetInsideAbove())
                        numInsideAbove++;
                    else
                        numNotInsideAbove++;
                    if (j != i+1) {
                        degenDms.SetDegeneratedFinal(DGM_IGNORE);
                        uregion->PutSegment(segments, j-1, degenDms);
                    }
                }

                if (MRA_DEBUG)
                    cerr << "InURegionEmbedded() degen-magic-f result "
                         << numInsideAbove
                         << " "
                         << numNotInsideAbove
                         << endl;

                if (numInsideAbove == numNotInsideAbove) {
                    dms.SetDegeneratedFinal(DGM_IGNORE);
                } else if (numInsideAbove == numNotInsideAbove+1) {
                    dms.SetDegeneratedFinal(DGM_INSIDEABOVE);
                } else if (numInsideAbove+1 == numNotInsideAbove) {
                    dms.SetDegeneratedFinal(DGM_NOTINSIDEABOVE);
                } else {
                    cerr << "segment ("
                         << dms.GetInitialStartX()
                         << ", " << dms.GetInitialStartY()
                         << ")-(" << dms.GetInitialEndX()
                         << ", " << dms.GetInitialEndY()
                         << ") / (" << dms.GetFinalStartX()
                         << ", " << dms.GetFinalStartY()
                         << ")-(" << dms.GetFinalEndX()
                         << ", " << dms.GetFinalEndY()
                         << ") incorrectly degenerated "
                         << "in final instant"
                         << endl;

                    delete uregion;
                    return 0;
                }
            } else
                dms.SetDegeneratedFinal(DGM_NONE);
        }

        uregion->PutSegment(segments, i, dms);
    }

   if (MRA_DEBUG)
        for (int i = 0; i < uregion->GetSegmentsNum(); i++) {
            const MSegmentData *dms;

            uregion->GetSegment(segments, i, dms);

            cerr << "InURegionEmbedded() resulting segment #"
                 << i
                 << ": "
                 << dms->ToString()
                 << endl;
        }

    return uregion;
}

/*
1.1.1 Function ~OutURegionEmbedded()~

*/
static ListExpr OutURegionEmbedded(
    const URegionEmb* ur,
    DBArray<MSegmentData>* segments) {

    if (MRA_DEBUG) 
         cerr << "OutURegionEmbedded() called" << endl;

/*
Conversion to list representation is straightforward. Just loop through
faces, cylces and segments and make sure that it is realised when one of
these changes.

*/

    int num = ur->GetSegmentsNum();

    ListExpr faces = nl->TheEmptyList();
    ListExpr facesLastElem = faces;

    ListExpr face = nl->TheEmptyList();
    ListExpr faceLastElem = face;
    ListExpr cycle = nl->TheEmptyList();
    ListExpr cycleLastElem = cycle;

    for (int i = 0; i < num; i++) {
        if (MRA_DEBUG)
            cerr << "OutURegionEmbedded() segment #" << i << endl;

        const MSegmentData* dms;
        ur->GetSegment(segments, i, dms);

        if (MRA_DEBUG)
            cerr << "OutURegionEmbedded() returned, dms=" << dms << endl;

        if (MRA_DEBUG)
            cerr << "OutURegionEmbedded() point is "
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
            if (MRA_DEBUG) cerr << "OutURegionEmbedded() new cycle" << endl;
            cycle = nl->OneElemList(p);
            cycleLastElem = cycle;
        } else {
            if (MRA_DEBUG)
                cerr << "OutURegionEmbedded() existing cycle" << endl;
            cycleLastElem = nl->Append(cycleLastElem, p);
        }

        const MSegmentData *nextDms;
        if (i < num-1) ur->GetSegment(segments, i+1, nextDms);

        if (i == num-1
            || dms->GetCycleNo() != nextDms->GetCycleNo()
            || dms->GetFaceNo() != nextDms->GetFaceNo()) {
            if (MRA_DEBUG)
                cerr << "OutURegionEmbedded() end of cycle" << endl;

            if (face == nl->TheEmptyList()) {
                if (MRA_DEBUG)
                    cerr << "OutURegionEmbedded() new face" << endl;
                face = nl->OneElemList(cycle);
                faceLastElem = face;
            } else {
                if (MRA_DEBUG)
                    cerr << "OutURegionEmbedded() existing face" << endl;
                faceLastElem = nl->Append(faceLastElem, cycle);
            }

            if (i == num-1
                || dms->GetFaceNo() != nextDms->GetFaceNo()) {
                if (MRA_DEBUG)
                    cerr << "OutURegionEmbedded() end of face" << endl;
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
                            SetWord((void*) &ur->timeInterval.start)),
                OutDateTime(nl->TheEmptyList(),
                            SetWord((void*) &ur->timeInterval.end)),
                nl->BoolAtom(ur->timeInterval.lc),
                nl->BoolAtom(ur->timeInterval.rc)),
            faces);

    return res;
}

/*
1 Data type ~uregion~

1.1 Class ~URegion~

1.1.1 Class definition

The class definition has been moved to ~MovingRegionAlgebra.h~.

1.1.1 Constructors

*/

URegion::URegion(unsigned int n) :
    SpatialTemporalUnit<Region, 3>(true), 
    segments(n) {

    if (MRA_DEBUG)
        cerr << "URegion::URegion() #1 called" 
             << endl;
}

URegion::URegion(int i, MRegion& mr) {
  
  assert( mr.IsDefined() );
  assert( i>=0 );
  assert( i<mr.GetNoComponents() );

  // get the appropriate URegionEmb from MRegion::units
  const URegionEmb* OrigURemb;
  mr.Get( i, OrigURemb );
  uremb = *OrigURemb;
  uremb.SetSegmentsNum(0);
  uremb.SetStartPos(0);
  segments = DBArray<MSegmentData>(0);

  // copy timeInterval from URegionEmb to URegion
  timeInterval = uremb.timeInterval;
  
  // copy MSegmentData
  const int startSeg = OrigURemb->GetStartPos();
  const int numSegs  = OrigURemb->GetSegmentsNum();

  for(int s=0 ; s<numSegs ; s++)
    {
      const MSegmentData *segment;
      mr.GetMSegmentData()->Get( s+startSeg, segment );
      uremb.PutSegment(&segments, s, *segment, true);
    }
  SetDefined( true );
}

URegionEmb& URegionEmb::operator=(const URegionEmb& U) {
  segmentsStartPos = U.segmentsStartPos;
  segmentsNum = U.segmentsNum;
  bbox = U.bbox;
  timeInterval = U.timeInterval;
  return *this;
}

/*
1.1.1 Methods for database operators

1.1.1.1 Method ~BoundingBox()~

*/
const Rectangle<3> URegion::BoundingBox() const {
    if (MRA_DEBUG) cerr << "URegion::BoundingBox() called" << endl;

    return uremb.BoundingBox();
}

/*
1.1.1.1 Method ~At()~

This method is not part of the bachelor thesis but a stub is required to make
~URegion~ non-abstract. To assure that accidential calls of this method are
recognized, its body contains a failed assertion.

*/
bool URegion::At(const Region& val, TemporalUnit<Region>& result) const {
    if (MRA_DEBUG) cerr << "URegion::At() called" << endl;

    assert(false);
}

/*
1.1.1.1 Method ~Passes()~

This method is not part of the bachelor thesis but a stub is required to make
~URegion~ non-abstract. To assure that accidential calls of this method are
recognized, its body contains a failed assertion.

*/
bool URegion::Passes(const Region& val) const {
    if (MRA_DEBUG) cerr << "URegion::At() called" << endl;

    assert(false);
}

/*
1.1.1.1 Method ~TemporalFunction()~

*/
void URegion::TemporalFunction(const Instant& t, 
                               Region& res, 
                               bool ignoreLimits ) const {
    if (MRA_DEBUG)
        cerr << "URegion::TemporalFunction() called" << endl;

    uremb.TemporalFunction(&segments, t, res, ignoreLimits);
}

/*
1.1.1 Methods for algebra integration

1.1.1.1 Method ~Clone()~

*/
URegion* URegion::Clone(void) const {
    if (MRA_DEBUG) cerr << "URegion::Clone() called" << endl;

    URegion* res = new URegion( (unsigned int)0 );
    res->CopyFrom(this);
    res->SetDefined( TemporalUnit<Region>::defined );
    return res;
}

/*
1.1.1.1 Method ~CopyFrom()~

*/
void URegion::CopyFrom(const StandardAttribute* right) {
    if (MRA_DEBUG) cerr << "URegion::CopyFrom() called" << endl;

    const URegion* ur = (const URegion*) right;

    *this = *ur;
    this->SetDefined( ur->IsDefined() );
}

/*
Assignment operator

*/

 URegion& URegion::operator= ( const URegion& U) {

   if(!U.IsDefined())
     {
       uremb = U.uremb;
       segments = U.segments;
       SetDefined(false);
       return *this;
     }
   timeInterval = U.timeInterval; // copy units copy of deftime!   
   uremb = U.uremb;      // copy bbox, timeInterval, segmentsNum
   uremb.SetStartPos(0); // set uremb.segmentsStartPos = 0

   int start = U.uremb.GetStartPos();
   int numsegs = U.uremb.GetSegmentsNum();
   segments.Clear();
   segments.Resize( U.uremb.GetSegmentsNum() );
   for( int i=0 ; i<numsegs ; i++ )
     {// copy single movingsegment
       const MSegmentData* seg;
       U.segments.Get(i+start, seg); 
       segments.Put(i, *seg);       
     }
   
   return *this;
 }

/*
1.1.1.1 ~DBArray~ access

*/

int URegion::NumOfFLOBs() const {
    if (MRA_DEBUG) cerr << "URegion::NumOfFLOBs() called" << endl;

    return 1;
}

FLOB* URegion::GetFLOB(const int i) {
    if (MRA_DEBUG) cerr << "URegion::GetFLOB() called" << endl;

    assert(i == 0);

    return &segments;
}

/*
1.1 Algebra integration

1.1.1 Function ~OutURegion()~

*/
static ListExpr OutURegion(ListExpr typeInfo, Word value) {
     if (MRA_DEBUG) 
         cerr << "OutURegion() called" << endl;

    URegion* ur = (URegion*) value.addr;

    // check for undefined value
    if( !ur->IsDefined() )
      return (nl->SymbolAtom("undef"));

    return
        OutURegionEmbedded(
            ur->GetEmbedded(),
            (DBArray<MSegmentData>*) ur->GetFLOB(0));
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


    URegion* ur = new URegion((unsigned int)0);

    // Check for indefined value
    if ( nl->IsAtom( instance ) && nl->AtomType( instance ) == SymbolType 
         && nl->SymbolValue( instance ) == "undef" )
      {
        ur->SetDefined(false);
        correct = true;
        return SetWord ( ur );
      }

    URegionEmb* uremb = 
        InURegionEmbedded(
            instance,
            errorPos,
            errorInfo,
            (DBArray<MSegmentData>*) ur->GetFLOB(0),
            0);

    if (!uremb) {
        cerr << "Could not grok list representation of region unit."
             << endl;
        delete ur;
        correct = false;
        return SetWord(Address(0));
    }

    ur->timeInterval = uremb->timeInterval;
    ur->SetEmbedded(uremb);
    ur->SetDefined(true);

    delete uremb;

    correct = true;
    return SetWord(Address(ur));
}

/*
1.1.1 Function ~URegionProperty()~

*/
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
1.1.1 Function ~CreateURegion()~

*/

static Word CreateURegion(const ListExpr typeInfo) {
    if (MRA_DEBUG) cerr << "CreateURegion() called" << endl;

    return (SetWord(new URegion((unsigned int)0)));
}

/*
1.1.1 Function ~DeleteURegion()~

*/
static void DeleteURegion(const ListExpr typeInfo, Word& w) {
    if (MRA_DEBUG) cerr << "DeleteURegion() called" << endl;

    URegion *ur = (URegion*)w.addr;
    delete ur;
    w.addr = 0;
}

/*
1.1.1 Function ~CloseURegion()~

*/
static void CloseURegion(const ListExpr typeInfo, Word& w) {
    if (MRA_DEBUG) cerr << "CloseURegion() called" << endl;

    delete (URegion*) w.addr;
    w.addr = 0;
}

/*
1.1.1 Function ~CloneURegion()~

*/
static Word CloneURegion(const ListExpr typeInfo, const Word& w) {
    if (MRA_DEBUG) cerr << "CloneURegion() called" << endl;

    return SetWord(((URegion*)w.addr)->Clone());
}

/*
1.1.1 Function ~CastURegion()~

*/
static void* CastURegion(void* addr) {
    if (MRA_DEBUG) cerr << "CastURegion() called" << endl;

    return new (addr) URegion;
}

/*
1.1.1 Function ~SizeOfURegion()~

*/
static int SizeOfURegion() {
    if (MRA_DEBUG) cerr << "SizeOfURegion() called" << endl;

    return sizeof(URegion);
}

/*
1.1.1 Function ~OpenURegion()~

*/
bool OpenURegion(SmiRecord& rec,
                 size_t& offset,
                 const ListExpr typeInfo,
                 Word& w) {
    if (MRA_DEBUG) cerr << "OpenURegion() called" << endl;

    w = SetWord(Attribute::Open(rec, offset, typeInfo));

    return true;
}

/*
1.1.1 Function ~SaveURegion()~

Makes sense on ~URegion~ instances with own segment storage only and will
run into failed assertion for other instances.

*/
static bool SaveURegion(SmiRecord& rec,
                        size_t& offset,
                        const ListExpr typeInfo,
                        Word& w) {
    if (MRA_DEBUG) cerr << "SaveURegion() called" << endl;

    URegion* ur = static_cast<URegion*> (w.addr);
    Attribute::Save(rec, offset, typeInfo, ur);

    return true;
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
    OpenURegion, 
    SaveURegion,
    CloseURegion,
    CloneURegion,
    CastURegion,
    SizeOfURegion,
    CheckURegion);

/*
1 Data type ~movingregion~

1.1 Class ~MRegion~

1.1.1 Class definition

The class definition has been moved to ~MovingRegionAlgebra.h~.

1.1.1 Private methods

1.1.1.1 Method ~IntersectionRP()~

*/
void MRegion::IntersectionRP(
    MPoint& mp,
    MPoint& res,
    RefinementPartitionOrig<
        MRegion,
        MPoint,
        URegionEmb,
        UPoint>& rp,
    bool merge) {

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

        const URegionEmb* ur;
        const UPoint* up;

        Get(urPos, ur);
        mp.Get(upPos, up);

        if (MRA_DEBUG)
            cerr << "MRegion::IntersectionRP() both elements present"
                 << endl;

        ur->RestrictedIntersection(
            &msegmentdata, *up, *iv, res, pending, merge);
    }

    if (pending) {
        if (!(nearlyEqual(pending->timeInterval.start.ToDouble(),
                          pending->timeInterval.end.ToDouble())
              && (!pending->timeInterval.lc
                  || !pending->timeInterval.rc))) {
            res.Add(*pending);
        }

        delete pending;
    }

    if (MRA_DEBUG)
        cerr << "MRegion::IntersectionRP() res.IsEmpty()="
             << res.IsEmpty()
             << endl;
}

/*
1.1.1.1 Method ~InsideAddUBool()~

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

    if (MRA_DEBUG) 
        cerr << "MRegion::InsideAddUBool() adding "
             << setprecision(20)
             << start.ToString()
             << " ("
             << starttime
             << ") " 
             << end.ToString()
             << " ("
             << endtime
             << ") "
             << lc
             << " "
             << rc 
             << endl;

    if (pending) {
        if (MRA_DEBUG) {
            cerr << "MRegion::InsideAddUBool() pending end="
                 << pending->timeInterval.end.ToString()
                 << " rc="
                 << pending->timeInterval.rc
                 << endl;
            cerr << "MRegion::InsideAddUBool() current start="
                 << start.ToString()
                 << " lc="
                 << lc
                 << endl;
        }

        if (nearlyEqual(starttime, pending->timeInterval.end.ToDouble())
            && (pending->timeInterval.rc || lc)
            && pending->constValue.GetBoolval() == value) {

            if (MRA_DEBUG)
                cerr << "MRegion::InsideAddUBool() connecting"
                     << endl;

            if (pending->timeInterval.start == pending->timeInterval.end) {
                Interval<Instant> iv(start,
                                     end,
                                     true,
                                     rc);

                if (MRA_DEBUG)
                    cerr << "MRegion::InsideAddUBool() connecting #1 to "
                         << iv.start.ToString()
                         << " "
                         << iv.end.ToString()
                         << " "
                         << iv.lc
                         << " "
                         << iv.rc
                         << endl;

                delete pending;
                pending = new UBool(iv, bv);
            } else if (starttime == endtime) {
                Interval<Instant> iv(start,
                                     end,
                                     pending->timeInterval.lc,
                                     true);

                if (MRA_DEBUG)
                    cerr << "MRegion::InsideAddUBool() connecting #2 to "
                         << iv.start.ToString()
                         << " "
                         << iv.end.ToString()
                         << " "
                         << iv.lc
                         << " "
                         << iv.rc
                         << endl;

                delete pending;
                pending = new UBool(iv, bv);
            } else {
                Interval<Instant> iv(pending->timeInterval.start,
                                     end,
                                     pending->timeInterval.lc,
                                     rc);

                if (MRA_DEBUG)
                    cerr << "MRegion::InsideAddUBool() connecting #3 to "
                         << iv.start.ToString()
                         << " "
                         << iv.end.ToString()
                         << " "
                         << iv.lc
                         << " "
                         << iv.rc
                         << endl;

                delete pending;
                pending = new UBool(iv, bv);
            }
        } else {
            if (MRA_DEBUG)
                cerr << "MRegion::InsideAddUBool() not connecting, pending is "
                     << pending->timeInterval.start.ToString()
                     << " "
                     << pending->timeInterval.end.ToString()
                     << " "
                     << pending->timeInterval.lc
                     << " "
                     << pending->timeInterval.rc
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
1.1.1 Constructors

*/

MRegion::MRegion(const int n) :
    Mapping<URegionEmb, Region>(n),
    msegmentdata(n) {

    if (MRA_DEBUG)
        cerr << "MRegion::MRegion(int) called" << endl;
}

MRegion::MRegion(MPoint& mp, Region& r) :
    Mapping<URegionEmb, Region>(0),
    msegmentdata(0) {

    if (MRA_DEBUG)
        cerr << "MRegion::MRegion(MPoint, Region) called"
             << endl;

    r.LogicSort();

    for (int i = 0; i < mp.GetNoComponents(); i++) {
        const UPoint *up;
        
        mp.Get(i, up);
            
        if (MRA_DEBUG)
            cerr << "MRegion::MRegion(MPoint, Region) i="
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
        
        URegionEmb
            ur(&msegmentdata,
               up->timeInterval,
               r,
               msegmentdata.Size());
        Add(ur);
    }
}

MRegion::MRegion(MPoint& mp, Region& r,int dummy) :
    Mapping<URegionEmb, Region>(0),
    msegmentdata(0) {

    if (MRA_DEBUG)
        cerr << "MRegion::MRegion(MPoint, Region,int) called"
             << endl;
    r.LogicSort();
    bool isFirst = true;
    Coord lastX, lastY;
    Coord firstX, firstY;
    for (int i = 0; i < mp.GetNoComponents(); i++) {
        const UPoint *up;
        mp.Get(i, up);
        firstX = up->p0.GetX();
        firstY = up->p0.GetY();
        
        if( ! isFirst &&  (firstX !=lastX || firstY != lastY)){
            r.Translate(firstX-lastX, firstY-lastY);         
        }else{
            isFirst=false;
        }
        
        lastX = up->p1.GetX();
        lastY = up->p1.GetY();
        
        URegionEmb
            ur(&msegmentdata,
               up->timeInterval,
               r,
               msegmentdata.Size());
        // now ur represents a static region
        // if this is not the case, we change the final line
        Coord tx = lastX-firstX;
        Coord ty = lastY-firstY;
        if(tx!=0 || ty!=0){
            size_t usize = ur.GetSegmentsNum();
            const MSegmentData* mseg;
            for(size_t i=0;i<usize;i++){
                ur.GetSegment(&msegmentdata, i, mseg);
                MSegmentData dms(*mseg);
                dms.SetFinalStartX(dms.GetFinalStartX()+tx);
                dms.SetFinalStartY(dms.GetFinalStartY()+ty);
                dms.SetFinalEndX(dms.GetFinalEndX()+tx);
                dms.SetFinalEndY(dms.GetFinalEndY()+ty);
                ur.PutSegment(&msegmentdata, i, dms);
            }
        }
        // change the bounding box of ur
        Rectangle<3> rbb(ur.BoundingBox());
        double t[3];
        t[0] = tx;
        t[1] = ty;
        t[2] = 0.0;
        rbb = rbb.Translate(t);
        ur.SetBBox(ur.BoundingBox().Union(rbb));    
        
        r.Translate(tx,ty);
        Add(ur);
    }
}

/*
1.1.1 Attribute access methods

1.1.1.1 Method ~Get()~

*/
void MRegion::Get(const int i, const URegionEmb*& ur) const {
    if (MRA_DEBUG)
        cerr << "MRegion::Get() #2 called i=" 
             << i 
             << " (msegmentdata="
             << &msegmentdata
             << ")"
             << endl;
    
    Mapping<URegionEmb, Region>::Get(i, ur);
}

/*
1.1.1.1 Method ~GetMSegmentData()~

*/
const DBArray<MSegmentData>* MRegion::GetMSegmentData(void) {
    return &msegmentdata;
}

/*
Add an idependent ~URegion~ object to the moving region. The URegions moving 
segment is copied to the DBArrays for ~msegmentdata~ and ~units~.

*/

void MRegion::AddURegion(URegion& U ) {
  

  cout << "MRegion::AddURegion" << endl;
  U.Print(cout);

  if ( !U.IsDefined() )
    return;

  URegionEmb tmp;
  int start=0, end=0;

  StartBulkLoad();

  // copy adjusted U.uremb data to units
  start = msegmentdata.Size();
  end = U.uremb.GetSegmentsNum();
  tmp = *(U.GetEmbedded());
  tmp.SetStartPos(start);
  Add(tmp);
  
  // append U.segments to msegmentdata
  for(int i=0 ; i<end ; i++)
    {
      const MSegmentData* dms;
      U.segments.Get(i, dms);
      msegmentdata.Put(i+start, *dms);
    }
  EndBulkLoad();
}



/*
1.1.1 Methods for database operators

1.1.1.1 Method ~Intersection()~

*/
void MRegion::Intersection(MPoint& mp, MPoint& res) {
    if (MRA_DEBUG)
        cerr << "MRegion::Intersection() #2 called" << endl;

    res.Clear();

    RefinementPartitionOrig<
        MRegion,
        MPoint,
        URegionEmb,
        UPoint> rp(*this, mp);

    IntersectionRP(mp, res, rp, true);
}

/*
1.1.1.1 Method ~Inside()~

*/
void MRegion::Inside(MPoint& mp, MBool& res) {
    if (MRA_DEBUG) cerr << "MRegion::Inside() called" << endl;

    res.Clear();

/*
Use intersection algorithm and then see how the units in the result
match to the original units in ~mp~.

First, we have to create the refinement partition for ~mp~ and this
~MRegion~ instance.

*/
    RefinementPartitionOrig<
        MRegion,
        MPoint,
        URegionEmb,
        UPoint> rp(*this, mp);

/*
We are calling ~IntersectionRP()~ with value ~false~ for parameter ~merge~.
This means that the resulting ~URegionEmb~ instances in ~resMp~ are not
merged, ie. one ~URegionEmb~ instance's interval is entirely within one
interval in ~rp~.

*/
    MPoint resMp;
    IntersectionRP(mp, resMp, rp, false);

    int mpPos = 0;

/*
Required for ~InsideAddUBool()~.

*/
    UBool* pending = 0;

/*
Pass through all the intervals in ~rp~.

*/
    for (unsigned int rpPos = 0; rpPos < rp.Size(); rpPos++) {
        if (MRA_DEBUG)
            cerr << "MRegion::Inside() rpPos=" << rpPos 
                 << " mpPos=" << mpPos
                 << endl;

/*
Get the units for each interval in ~rp~.

*/
        Interval<Instant>* iv;
        int urPos;
        int upPos;

        rp.Get(rpPos, iv, urPos, upPos);

/*
We are only interested in the intervals, which are contained in ~mp~.
If an interval in ~rp~ is not in ~mp~, ~upPos~ is lower than $0$ and
we skip this interval.

*/
        if (upPos < 0) continue;

        double prevtime = iv->start.ToDouble();
        Instant prev(instanttype);
        prev.ReadFrom(prevtime);
        bool prev_c = !iv->lc;

/*
Check every ~upoint~ instance in the intersection,
which has an interval within the current interval.
If we reach an instance outside the current interval,
the ~for~ loop is left via a ~break~ statement.

*/ 
        for (; mpPos < resMp.GetNoComponents(); mpPos++) {
            if (MRA_DEBUG)
                cerr << "MRegion::Inside()   mpPos=" << mpPos
                     << endl;
            
/*
Get the current ~upoint~ instance.

*/              
            const UPoint *up;
            resMp.Get(mpPos, up);
            
            if (MRA_DEBUG)
                cerr << "MRegion::Inside()     rp iv=["
                     << iv->start.ToString()
                     << " "
                     << iv->end.ToString()
                     << " "
                     << iv->lc
                     << " "
                     << iv->rc
                     << "] up iv=["
                     << up->timeInterval.start.ToString()
                     << " "
                     << up->timeInterval.end.ToString()
                     << " "
                     << up->timeInterval.lc
                     << " "
                     << up->timeInterval.rc
                     << "]"
                     << endl;
            
/*
Is this ~upoint~ instance really within the current interval?

*/
            if (up->timeInterval.start.Compare(&iv->end) > 0
                || (up->timeInterval.start.Compare(&iv->end) == 0
                    && up->timeInterval.lc
                    && !iv->rc)) {
/*
No, we reached the end of the current interval. We add a ~ubool~ instance
with value ~false~ to the result, which closes the gap between the previously
added ~ubool~ instance and the end of the current interval (only if 
applicable).

*/

                if (MRA_DEBUG)
                    cerr << "MRegion::Inside()     end of unit"
                         << endl;
                
                if (prev.Compare(&iv->end) < 0) {
                    if (MRA_DEBUG)
                        cerr << "MRegion::Inside()     closing gap #2a"
                             << endl;
                    
                    InsideAddUBool(res,
                                   prevtime,
                                   iv->end.ToDouble(),
                                   !prev_c,
                                   iv->rc,
                                   false,
                                   prevtime,
                                   prev_c,
                                   pending);
                    prev.ReadFrom(prevtime);
                }
                
/*
Break out of the loop iterating through the ~upoint~ instances in the
intersection.

*/
                break;
            } else {
/*
Yes, the current ~upoint~ instance is still within the current interval.

*/
                if (MRA_DEBUG)
                    cerr << "MRegion::Inside()     inside unit"
                         << endl;
                
/*
Is there a gap between the previously added ~ubool~ instance and the
start of the interval of the current ~upoint~ instance? If so, close 
the gap by adding an ~ubool~ instance with an appropriate interval
and value ~false~.

*/
                if (up->timeInterval.start.Compare(&prev) > 0) {
                    if (MRA_DEBUG)
                        cerr << "MRegion::Inside()     closing gap #1a, prev="
                             << setprecision(20)
                             << prevtime
                             << endl;

                    InsideAddUBool(res,
                                   prevtime,
                                   up->timeInterval.start.ToDouble(),
                                   !prev_c,
                                   !up->timeInterval.lc,
                                   false,
                                   prevtime,
                                   prev_c,
                                   pending);
                    prev.ReadFrom(prevtime);
                } else if (up->timeInterval.start.Compare(&prev) == 0
                           && !up->timeInterval.lc
                           && prev_c) {
                    if (MRA_DEBUG)
                        cerr << "MRegion::Inside()     closing gap #1b, prev="
                             << setprecision(15)
                             << prevtime
                             << endl;
                    
                    InsideAddUBool(res,
                                   up->timeInterval.start.ToDouble(),
                                   up->timeInterval.start.ToDouble(),
                                   true,
                                   true,
                                   false,
                                   prevtime,
                                   prev_c,
                                   pending);
                    prev.ReadFrom(prevtime);
                }
                    
/*
Add a ~ubool~ instance with the interval of the current ~upoint~ instance
and value ~true~.

*/
                InsideAddUBool(res,
                               up->timeInterval.start.ToDouble(),
                               up->timeInterval.end.ToDouble(),
                               up->timeInterval.lc,
                               up->timeInterval.rc,
                               true,
                               prevtime,
                               prev_c,
                               pending);
                prev.ReadFrom(prevtime);
            }
        }

        if (mpPos == resMp.GetNoComponents()) {
            if (MRA_DEBUG)
                cerr << "MRegion::Inside()   end of units reached before"
                     << endl;

            if (prev.Compare(&iv->end) < 0) {
                if (MRA_DEBUG)
                    cerr << "MRegion::Inside()     closing gap #3a"
                         << endl;

                InsideAddUBool(res,
                               prevtime,
                               iv->end.ToDouble(),
                               !prev_c,
                               iv->rc,
                               false,
                               prevtime,
                               prev_c,
                               pending);
                prev.ReadFrom(prevtime);
            } else if (prev.Compare(&iv->end) == 0
                       && !prev_c
                       && iv->rc) {
                if (MRA_DEBUG)
                    cerr << "MRegion::Inside()     closing gap #3b"
                         << endl;

                InsideAddUBool(res,
                               iv->end.ToDouble(),
                               iv->end.ToDouble(),
                               true,
                               true,
                               false,
                               prevtime,
                               prev_c,
                               pending);
                prev.ReadFrom(prevtime);
            }
        }
    }
    
/*
Since ~InsideAddUBool()~ is trying to merge adjacent ~ubool~ instances
with identical value, there may be an ~ubool~ instance left in ~pending~,
which needs to be added to the result.

*/
    if (pending) {
        if (MRA_DEBUG)
            cerr << "MRegion::Inside() adding pending" << endl;
        
        res.Add(*pending);
        delete pending;
    }

    res.SetDefined(!res.IsEmpty());
}

/*
1.1.1.1 Method ~AtInstant()~

Method ~Mapping<Unit, Alpha>::AtInstant()~ is not sufficient since it
does not deal with setting the unit's segment data DBArray.

*/

void MRegion::AtInstant(Instant& t, Intime<Region>& result) {
    if (MRA_DEBUG) cerr << "MRegion::AtInstant() called" << endl;

    assert(IsOrdered() && t.IsDefined());

    int pos = Position(t );

    if( pos == -1 )
        result.SetDefined(false);
    else {
        const URegionEmb* posUnit;
        Get(pos, posUnit);

        posUnit->TemporalFunction(&msegmentdata, t, result.value);
        result.instant.CopyFrom(&t);
        result.SetDefined(result.value.Size() > 0);
  }
}

/*
1.1.1.1 Method ~Initial()~

Method ~Mapping<Unit, Alpha>::Initial()~ is not sufficient since it
does not deal with setting the unit's segment data DBArray.

*/

void MRegion::Initial(Intime<Region>& result) {
    if (MRA_DEBUG) cerr << "MRegion::Initial() called" << endl;

    assert(IsOrdered());

    if (IsEmpty()) {
        result.SetDefined(false);
        return;
    }

    const URegionEmb* unit;
    Get(0, unit);

    if (!unit->timeInterval.lc) {
        result.SetDefined(false);
        return;
    }
        
    result.SetDefined(true);
    unit->TemporalFunction(
        &msegmentdata, 
        unit->timeInterval.start, 
        result.value);
    result.instant.CopyFrom(&unit->timeInterval.start);
    result.SetDefined(result.value.Size() > 0);
}

/*
1.1.1.1 Method ~Final()~

Method ~Mapping<Unit, Alpha>::Final()~ is not sufficient since it
does not deal with setting the unit's segment data DBArray.

*/

void MRegion::Final(Intime<Region>& result) {
    if (MRA_DEBUG) cerr << "MRegion::Final() called" << endl;

    assert(IsOrdered());

    if (IsEmpty()) {
        result.SetDefined(false);
        return;
    }

    const URegionEmb* unit;
    Get(GetNoComponents()-1, unit);
        
    if (!unit->timeInterval.rc) {
        result.SetDefined(false);
        return;
    }
        
    result.SetDefined(true);
    unit->TemporalFunction(
        &msegmentdata, 
        unit->timeInterval.end, 
        result.value);
    result.instant.CopyFrom(&unit->timeInterval.end);
    result.SetDefined(result.value.Size() > 0);
}

/*
1.1.1.1 Method ~Traversed()~

*/

#ifdef MRA_TRAVERSED
void MRegion::Traversed(Region& res) {
    if (MRA_DEBUG) cerr << "MRegion::Traversed() called" << endl;

/*
Methods relating to operator ~traversed~ are currently being developed.
In the meantime, we provide a stub with a failed assertion to recognize
accidential calls of this method.

*/
    assert(false);
}
#endif // MRA_TRAVERSED

/*
1.1.1 Methods for algebra integration

1.1.1.1 Method ~Clone()~

*/

MRegion* MRegion::Clone(void) const {
    if (MRA_DEBUG) cerr << "MRegion::Clone() called" << endl;

    MRegion* res = new MRegion(0);
    res->CopyFrom(this);

    return res;
}

/*
1.1.1.1 Method ~CopyFrom()~

*/
void MRegion::CopyFrom(const StandardAttribute* right) {
    if (MRA_DEBUG) 
        cerr << "MRegion::CopyFrom() called, this="
             << this
             << ", &msegmentdata=" 
             << &msegmentdata
             << endl;

    MRegion* mr = (MRegion*) right;

    if (MRA_DEBUG) 
        cerr << "MRegion::CopyFrom() called, &mr->msegmentdata=" 
             << &mr->msegmentdata
             << endl;

/*
~InMRegion()~ sorts the region units, therefore the following assumption
should hold.

*/
    assert(mr->IsOrdered());

    Clear();

/*
Copy the units and assure that their pointer to the ~DBAarry~ containing
their moving segments points to this instance's ~DBArray~.

*/
    StartBulkLoad();
    for(int i = 0; i < mr->GetNoComponents(); i++) {
        const URegionEmb* ur;
        mr->Get(i, ur);
        Add(*ur);
    }
    EndBulkLoad(false);

/* 
Copy the moving segments.

*/
    msegmentdata.Clear();
    msegmentdata.Resize(mr->msegmentdata.Size());
    for (int i = 0; i < mr->msegmentdata.Size(); i++) {
        const MSegmentData* dms;
        mr->msegmentdata.Get(i, dms);

        if (MRA_DEBUG) 
            cerr << "MRegion::CopyFrom() segment " 
                 << i 
                 << ": initial=[" 
                 << dms->GetInitialStartX() 
                 << " " << dms->GetInitialStartY() 
                 << " " << dms->GetInitialEndX() 
                 << " " << dms->GetInitialEndY()
                 << "] final=["
                 << dms->GetFinalStartX() 
                 << " " << dms->GetFinalStartY() 
                 << " " << dms->GetFinalEndX() 
                 << " " << dms->GetFinalEndY()
                 << "]"
                 << endl;

        msegmentdata.Put(i, *dms);
    }
}

/*
1.1.1.1 ~DBArray~ access

*/

int MRegion::NumOfFLOBs() const {
    if (MRA_DEBUG) cerr << "MRegion::NumOfFLOBs() called" << endl;

    return 2;
}

FLOB* MRegion::GetFLOB(const int i) {
    if (MRA_DEBUG) cerr << "MRegion::GetFLOB() called" << endl;

    assert(i == 0 || i == 1);

    return
        i == 0
        ? Mapping<URegionEmb, Region>::GetFLOB(0)
        : &msegmentdata;
}

/*
1.1.1 Unit testing

*/

#ifdef MRA_UNITTEST
bool MRegion::Unittest2(int pos) {

    if (MRA_DEBUG)
        cerr << "MRegion::Unittest2() called pos="
             << pos
             << endl;

    if (pos < 0 || pos >= msegmentdata.Size()) return -1;

    const MSegmentData *dms;
    msegmentdata.Get(pos, dms);

    return dms->GetInsideAbove();
}
#endif // MRA_UNITTEST

/*
1.1 Algebra integration

1.1.1 Function ~MRegionProperty()~

*/

static ListExpr MRegionProperty() {
    if (MRA_DEBUG) cerr << "MRegionProperty() called" << endl;

    ListExpr listrep = nl->TextAtom();
    nl->AppendText(listrep,
                   "(u1 ... un) with ui uregion list representations "
                   "and n >= 1. Each ui is of format "
                   "(<interval> <face>*), where <interval> is "
                   "(<real> <real> <bool> <bool>) and where "
                   "<face> is (<outercycle> <holecycle>*), "
                   "where <outercycle> and <holecycle> are "
                   "(<real> <real> <real> <real>), representing "
                   "start X, start Y, end X and end Y values.");
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
    if (MRA_DEBUG) 
        cerr << "OutMRegion() called" << endl;

    MRegion* mr = (MRegion*) value.addr;

    if (mr->IsEmpty()) return (nl->TheEmptyList());

    assert(mr->IsOrdered());

    ListExpr l = nl->TheEmptyList();
    ListExpr lastElem;

    if (MRA_DEBUG) 
        cerr << "OutMRegion() #units=" << mr->GetNoComponents() << endl;

    for (int i = 0; i < mr->GetNoComponents(); i++) {
        if (MRA_DEBUG) 
            cerr << "OutMRegion() i=" << i << endl;

        const URegionEmb* ur;
        mr->Get(i, ur);

        ListExpr unitList = 
            OutURegionEmbedded(
                ur,
                (DBArray<MSegmentData>*) mr->GetFLOB(1));

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

        URegionEmb* ur =
            (URegionEmb*) InURegionEmbedded(
                first,
                errorPos,
                errorInfo,
                &mr->msegmentdata,
                mr->msegmentdata.Size());
        if (!ur) {
            correct = false;
            mr->Destroy();
            delete mr;
            return SetWord(Address(0));
        }
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

    w = SetWord(Attribute::Open(rec, offset, typeInfo));

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

    MRegion* mr = static_cast<MRegion*> (w.addr);
    Attribute::Save(rec, offset, typeInfo, mr);

    return true;
}

/*
1.1.1 Type constructor ~mregion~

*/
static Word CreateMRegion(const ListExpr typeInfo) {
  if (MRA_DEBUG) cerr << "CreateMRegion() called" << endl;
  return SetWord(new MRegion(0));
}
static void DeleteMRegion(const ListExpr typeInfo, Word& w) {
  if (MRA_DEBUG) cerr << "DeleteMRegion() called" << endl;
  ((MRegion*) w.addr)->Destroy();
  delete (MRegion*) w.addr;
  w.addr = 0;
}
static void CloseMRegion(const ListExpr typeInfo, Word& w) {
  if (MRA_DEBUG) cerr << "CloseMRegion() called" << endl;
  delete (MRegion*) w.addr;
  w.addr = 0;
}
static Word CloneMRegion(const ListExpr typeInfo, const Word& w) {
  if (MRA_DEBUG) cerr << "CloneMRegion() called" << endl;
  return SetWord(((MRegion*) w.addr)->Clone());
}
static void* CastMRegion(void* addr) {
  if (MRA_DEBUG) cerr << "CastMRegion() called" << endl;
  return new (addr) MRegion;
}
static int SizeOfMRegion() {
  if (MRA_DEBUG) cerr << "SizeOfMRegion() called" << endl;
  return sizeof(MRegion);
}

static TypeConstructor mregion(
    "movingregion",
    MRegionProperty,
    OutMRegion,
    InMRegion,
    0, 0, // SaveToList, RestoreFromList
    CreateMRegion, DeleteMRegion,
    OpenMRegion, SaveMRegion,
    CloseMRegion, CloneMRegion,
    CastMRegion,
    SizeOfMRegion,
    CheckMRegion);

/*
1.1 Helper function(s)

Create ~MPoint~ instance from intervals in units in ~MRegion~ instance
~mr~ and the constant point ~p~.

*/

static MPoint CreateMPointFromPoint(MRegion* mr, Point* p) {
    if (MRA_DEBUG) cerr << "CreateMPointFromPoint() called" << endl;

    MPoint mp(0);

    for (int i = 0; i < mr->GetNoComponents(); i++) {
        const URegionEmb* ur;

        mr->Get(i, ur);

        UPoint up(ur->timeInterval,
                  p->GetX(), p->GetY(),
                  p->GetX(), p->GetY());

        mp.Add(up);
    }

    return mp;
}

/*
1 Operator definition

1.1 Type mapping functions

1.1.1 Generic

Used by ~intersection~:

*/

static ListExpr MPointMRegionToMPointTypeMap(ListExpr args) 
{
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

#ifdef MRA_TRAVERSED
static ListExpr MRegionToRegionTypeMap(ListExpr args) {
    if (MRA_DEBUG)
        cerr << "MRegionToRegionTypeMap() called" << endl;

    if (nl->ListLength(args) == 1
        && nl->IsEqual(nl->First(args), "movingregion"))
        return nl->SymbolAtom("region");
    else
        return nl->SymbolAtom("typeerror");
}
#endif // MRA_TRAVERSED

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
Used by ~atinstant~:

*/
static ListExpr AtInstantTypeMap(ListExpr args) {
    if (MRA_DEBUG)
        cerr << "AtInstantTypeMap() called" << endl;

    if (nl->ListLength(args) == 2
        && nl->IsEqual(nl->First(args), "movingregion")
        && nl->IsEqual(nl->Second(args), "instant"))
        return nl->SymbolAtom("intimeregion");
    else if (nl->ListLength(args) == 2
        && nl->IsEqual(nl->First(args), "uregion")
        && nl->IsEqual(nl->Second(args), "instant"))
        return nl->SymbolAtom("intimeregion");
    else
        return nl->SymbolAtom("typeerror");
}

/*
Used by ~mraprec~:

*/

#ifdef MRA_PREC
static ListExpr MraprecTypeMap(ListExpr args) {
    if (MRA_DEBUG)
        cerr << "MraprecTypeMap() called" << endl;

    if (nl->ListLength(args) == 2
        && nl->IsEqual(nl->First(args), "real")
        && nl->IsEqual(nl->Second(args), "real"))
        return nl->SymbolAtom("bool");
    else
        return nl->SymbolAtom("typeerror");
}
#endif // MRA_PREC

/*
Type mapping of the ~move~ operator:

*/

static ListExpr MoveTypeMap(ListExpr args){
    if (MRA_DEBUG) cout << "MOveTypeMap called " << endl;
    if(nl->ListLength(args)!=2){
        ErrorReporter::ReportError("invalid number of arguments");
        return nl->SymbolAtom("typeerror");
    }
    if(!nl->IsEqual(nl->First(args),"mpoint")){
        ErrorReporter::ReportError("mpoint as first argument required");
        return nl->SymbolAtom("typeerror");
    }
    if(!nl->IsEqual(nl->Second(args),"region")){
        ErrorReporter::ReportError("region as second argument required");
        return nl->SymbolAtom("typeerror");
    }
    if (MRA_DEBUG) cout << "Typemap returns movingregion" << endl;
    return nl->SymbolAtom("movingregion");
}

/*
Used by ~bbox~:

*/

static ListExpr BboxTypeMap(ListExpr args) {
    if (MRA_DEBUG)
        cerr << "BboxTypeMap() called" << endl;

    if (nl->ListLength(args) == 1
        && nl->IsEqual(nl->First(args), "uregion"))
        return nl->SymbolAtom("rect3");
    else
        return nl->SymbolAtom("typeerror");
}

/*
Type mapping of the ~vertextrajectory~ operator:

*/

static ListExpr VertTrajectoryTypeMap(ListExpr args){
   if(nl->ListLength(args)!=1){
       ErrorReporter::ReportError("wrong number of arguments");
       return nl->SymbolAtom("typeerror");
   }
   ListExpr arg = nl->First(args);
   if(nl->IsEqual(arg,"uregion") || nl->IsEqual(arg,"movingregion")){
     return nl->SymbolAtom("line");
   }
   ErrorReporter::ReportError("uregion or movingregion"
                              " expected but got ");
   return nl->SymbolAtom("typeerror");
}


/*
Type mapping for the ~units~ operator:

*/

ListExpr MRAUnitsTypeMap( ListExpr args )
{
  if ( nl->ListLength(args) == 1 )
  {
    ListExpr arg1 = nl->First(args);

    if( nl->IsEqual( arg1, "movingregion" ) )
      return nl->TwoElemList(nl->SymbolAtom("stream"),
       nl->SymbolAtom("uregion"));
    
  }
  return nl->SymbolAtom("typeerror");
}

/*
1.1.1 For unit testing operators

*/

#ifdef MRA_UNITTEST
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
        return nl->SymbolAtom("bool");
    else
        return nl->SymbolAtom("typeerror");
}

static ListExpr Unittest3TypeMap(ListExpr args) {
    if (MRA_DEBUG)
        cerr << "Unittest3TypeMap() called" << endl;

    if (nl->ListLength(args) == 2
        && nl->IsEqual(nl->First(args), "mpoint")
        && nl->IsEqual(nl->Second(args), "mpoint"))
        return nl->SymbolAtom("rreal");
    else
        return nl->SymbolAtom("typeerror");
}
#endif // MRA_UNITTEST

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
Used by ~atinstant~:

*/

static int AtInstantSelect(ListExpr args) {
    if (MRA_DEBUG)
        cerr << "AtInstantSelect() called" << endl;

    if (nl->ListLength(args) == 2
        && nl->SymbolValue(nl->First(args)) == "movingregion"
        && nl->SymbolValue(nl->Second(args)) == "instant")
        return 0;
    else if (nl->ListLength(args) == 2
        && nl->SymbolValue(nl->First(args)) == "uregion"
        && nl->SymbolValue(nl->Second(args)) == "instant")
        return 1;
    else
        return -1;
}

/*
Used by ~vertrajectory~:

*/

static int VertTrajectorySelect(ListExpr args){
   ListExpr arg = nl->First(args);
   if(nl->IsEqual(arg,"uregion"))
      return 0;
   else
      return 1;
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
    MPoint* res = (MPoint*) result.addr;

    try {
        MPoint* mp = (MPoint*) args[0].addr;
        MRegion* mr = (MRegion*) args[1].addr;

        if (!mp->IsDefined() || !mr->IsDefined())
            res->SetDefined(false);
        else 
            mr->Intersection(*mp, *res);
    } catch (invalid_argument& e) {
        cerr << "-----------------------------------------------------------"
             << endl
             << "An error occured during the intersection operation:"
             << endl
             << e.what()
             << endl
             << "-----------------------------------------------------------"
             << endl;
        res->SetDefined(false);
    }

    return 0;
}

static int InsideValueMap(Word* args,
                          Word& result,
                          int message,
                          Word& local,
                          Supplier s) {
    if (MRA_DEBUG) cerr << "InsideValueMap() called" << endl;

    result = qp->ResultStorage(s);
    MBool* res = (MBool*) result.addr;

    try {
        MPoint* mp = (MPoint*) args[0].addr;
        MRegion* mr = (MRegion*) args[1].addr;

        if (!mp->IsDefined() || !mr->IsDefined())
            res->SetDefined(false);
        else
            mr->Inside(*mp, *res);
     } catch (invalid_argument& e) {
        cerr << "-----------------------------------------------------------"
             << endl
             << "An error occured during the inside operation:"
             << endl
             << e.what()
             << endl
             << "-----------------------------------------------------------"
             << endl;
        ((MBool*) result.addr)->SetDefined(false);
    }

    return 0;
}

static int AtValueMap_MPoint(Word* args,
                             Word& result,
                             int message,
                             Word& local,
                             Supplier s) {
    if (MRA_DEBUG) cerr << "AtValueMap_MPoint() called" << endl;

    result = qp->ResultStorage(s);
    MPoint* res = (MPoint*) result.addr;

    try {
        MPoint* mp = (MPoint*) args[0].addr;
        Region* r = (Region*) args[1].addr;
        
        if (!mp->IsDefined() || !r->IsDefined())
            res->SetDefined(false);
        else {
            MRegion* mr = new MRegion(*mp, *r);

            mr->Intersection(*mp, *res);
        }
    } catch (invalid_argument& e) {
        cerr << "-----------------------------------------------------------"
             << endl
             << "An error occured during the at operation:"
             << endl
             << e.what()
             << endl
             << "-----------------------------------------------------------"
             << endl;
        ((MPoint*) result.addr)->SetDefined(false);
    }

    return 0;
}

static int AtValueMap_MRegion(Word* args,
                              Word& result,
                              int message,
                              Word& local,
                              Supplier s) {
    if (MRA_DEBUG) cerr << "AtValueMap_MRegion() called" << endl;

    result = qp->ResultStorage(s);
    MPoint* res = (MPoint*) result.addr;

    try {
        MRegion* mr = (MRegion*) args[0].addr;
        Point* p = (Point*) args[1].addr;

        if (!mr->IsDefined() || !p->IsDefined()) 
            res->SetDefined(false);
        else {
            MPoint mp = CreateMPointFromPoint(mr, p);

            mr->Intersection(mp, *res);
        }
    } catch (invalid_argument& e) {
        cerr << "-----------------------------------------------------------"
             << endl
             << "An error occured during the at operation:"
             << endl
             << e.what()
             << endl
             << "-----------------------------------------------------------"
             << endl;
        ((MPoint*) result.addr)->SetDefined(false);
    }

    return 0;
}

static int AtInstantValueMap_URegion(Word* args,
                                     Word& result,
                                     int message,
                                     Word& local,
                                     Supplier s) {
    if (MRA_DEBUG) cerr << "AtInstantValueMap_URegion() called" << endl;

    result = qp->ResultStorage(s);

    URegion* ur = (URegion*) args[0].addr;
    Instant* inst = (Instant*) args[1].addr;

    Intime<Region>* res = (Intime<Region>*) result.addr;

    if (ur->timeInterval.Contains(*inst)) {
        ur->TemporalFunction(*inst, res->value);
        res->instant.CopyFrom(inst);
        res->SetDefined(true);
    } else 
        res->SetDefined(false);
        
    return 0;
}

#ifdef MRA_TRAVERSED
static int TraversedValueMap(Word* args,
                             Word& result,
                             int message,
                             Word& local,
                             Supplier s) {
    if (MRA_DEBUG) cerr << "TraversedValueMap() called" << endl;

    result = qp->ResultStorage(s);
    Region* res = (Region*) result.addr;

    MRegion* mr = (MRegion*) args[0].addr;

    if (!mr->IsDefined()) 
        res->SetDefined(false);
    else
        mr->Traversed(*res);

    return 0;
}
#endif // MRA_TRAVERSED

#ifdef MRA_PREC
static int MraprecValueMap(Word* args,
                           Word& result,
                           int message,
                           Word& local,
                           Supplier s) {
    if (MRA_DEBUG) cerr << "MraprecValueMap() called" << endl;

    result = qp->ResultStorage(s);
    CcBool* res = (CcBool*) result.addr;

    CcReal* r1 = (CcReal*) args[0].addr;
    CcReal* r2 = (CcReal*) args[1].addr;

    if (!r1->IsDefined() || !r2->IsDefined()) 
        res->SetDefined(false);
    else {
        eps = r1->GetRealval();
        epsRelaxFactor = r2->GetRealval();
        res->Set(true, true);
    }

    return 0;
}
#endif // MRA_PREC

static int BboxValueMap(Word* args,
                           Word& result,
                           int message,
                           Word& local,
                           Supplier s) {
    if (MRA_DEBUG) cerr << "BBox() called" << endl;

    result = qp->ResultStorage(s);
    Rectangle<3>* res = (Rectangle<3>*) result.addr;

    URegion* ur = (URegion*) args[0].addr;

    if (!ur->IsDefined()) 
        res->SetDefined(false);
    else
        *res = ur->BoundingBox();

    return (0);
}

static int MoveValueMap(Word* args,
                        Word& result,
                        int message,
                        Word& local,
                        Supplier s) {
    cout << "MoveValueMap called" << endl;
    result = qp->ResultStorage(s);
    cout << "Step one ok "<< endl;
    MPoint* mp = (MPoint* ) args[0].addr;
    Region* reg = (Region*) args[1].addr;
    MRegion res(*mp,*reg,0);
    ((MRegion*)result.addr)->CopyFrom(&res);
    res.Destroy();
    return 0;
}

/*
Value mapping functions of operator ~units~

*/

struct MRAUnitsLocalInfo
{
  Word mWord;     // the address of the moving point/int/real value
  int unitIndex;  // current item index
};

int mraunitsvalmap(Word* args, Word& result, 
                   int message, Word& local, Supplier s)
{
  MRegion* m;
  URegion* unit;
  MRAUnitsLocalInfo *localinfo;

  switch( message )
  {
    case OPEN:

      localinfo = new MRAUnitsLocalInfo;
      localinfo->mWord = args[0];
      localinfo->unitIndex = 0;
      local = SetWord(localinfo);
      return 0;

    case REQUEST:

      if( local.addr == 0 )
        return CANCEL;
      localinfo = (MRAUnitsLocalInfo *) local.addr;
      m = (MRegion*)localinfo->mWord.addr;
      if( (0 <= localinfo->unitIndex) 
          && (localinfo->unitIndex < m->GetNoComponents()) )
        {
          unit = new URegion(localinfo->unitIndex++, *m);
          result = SetWord( unit );
          return YIELD;
        }
      return CANCEL;

    case CLOSE:

      if( local.addr != 0 )
        delete (MRAUnitsLocalInfo *)local.addr;
      return 0;
  }

  // should not happen 
  return -1;
}


/*
1.1.1 For unit testing operators

*/

#ifdef MRA_UNITTEST
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
    ((CcBool *)result.addr)->Set(
        true,
        ((MRegion*) args[0].addr)->Unittest2(
            ((CcInt*) args[1].addr)->GetIntval()));

    return 0;
}

static int Unittest3ValueMap(Word* args,
                             Word& result,
                             int message,
                             Word& local,
                             Supplier s) {
    if (MRA_DEBUG) cerr << "Unittest3ValueMap() called" << endl;

    result = qp->ResultStorage(s);
    RReal* res = (RReal *) result.addr;

    MPoint* mp1 = (MPoint*) args[0].addr;
    MPoint* mp2 = (MPoint*) args[1].addr;

    RefinementPartitionOrig<
        MPoint,
        MPoint,
        UPoint,
        UPoint> rp(*mp1, *mp2);

    res->Clear();
    res->StartBulkLoad();

    for (unsigned int i = 0; i < rp.Size(); i++) {
        Interval<Instant>* iv;
        int urPos;
        int upPos;

        rp.Get(i, iv, urPos, upPos);

        CcReal left(true, iv->start.ToDouble());
        CcReal right(true, iv->end.ToDouble());
        Interval<CcReal> resIv(left, right, iv->lc, iv->rc);

        res->Add(resIv);
    }

    res->EndBulkLoad(true);

    return 0;
}
#endif // MRA_UNITTEST

template <class T>
static int VertTrajectory_ValueMap(Word* args,
                                     Word& result,
                                     int message,
                                     Word& local,
                                     Supplier s) {
    result = qp->ResultStorage(s);
    T* ur = (T*) args[0].addr;
    const DBArray<MSegmentData>* segments = ur->GetMSegmentData();
    int size = segments->Size();
    Line* L = (Line*) result.addr;
    L->Clear();
    L->StartBulkLoad();
    const MSegmentData* mseg;
    cout << " insert " << size << " segments" << endl;
    HalfSegment hs;
    int edgeno = 0;
    for(int i=0;i<size;i++){
        segments->Get(i,mseg);
        Point P1(true,mseg->GetInitialStartX(),mseg->GetInitialStartY());
        Point P2(true,mseg->GetFinalStartX(),mseg->GetFinalStartY());
        hs.Set(true,P1,P2);
        hs.attr.edgeno = edgeno++;
        //cout << "insert " << hs << "to the line" << endl;
        (*L)+=hs;
        hs.SetLeftDomPoint(!hs.IsLeftDomPoint());
        (*L)+=hs; 
    } 
    L->EndBulkLoad();
    return 0;
}

/*
1.1 Value mapping arrays

*/

static ValueMapping atinstantvaluemap[] =
    { MappingAtInstant<MRegion, Region>,
      AtInstantValueMap_URegion };

static ValueMapping initialvaluemap[] =
    { MappingInitial<MRegion, URegionEmb, Region> };

static ValueMapping finalvaluemap[] =
    { MappingFinal<MRegion, URegionEmb, Region> };

static ValueMapping instvaluemap[] =
    { IntimeInst<Region> };

static ValueMapping valvaluemap[] =
    { IntimeVal<Region> };

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

static ValueMapping verttrajectoryvaluemap[] =
    { VertTrajectory_ValueMap<URegion>, 
      VertTrajectory_ValueMap<MRegion>};

#ifdef MRA_TRAVERSED
static ValueMapping traversedvaluemap[] =
    { TraversedValueMap };
#endif // MRA_TRAVERSED

/*
1.1 Operator specifications

*/

static const string atinstantspec  =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "  ( <text>(mregion instant) -> iregion, "
    "    (uregion instant) -> iregion,</text--->"
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
    "    <text>intersection( _ , _ )</text--->"
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
    "  ( <text>(mpoint region) -> mpoint, "
    "(mregion point) -> mpoint</text--->"
    "    <text>_ at _</text--->"
    "    <text>Restrict moving point to region or restrict moving region "
    "to point.</text--->"
    "    <text>mpoint1 at region1</text---> ) )";

static const string movespec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "  ( <text>mpoint x region -> mregion</text--->"
    "    <text>move( _ , _)</text--->"
    "    <text>Creates a moving region from the given region"
    "  using the mpoint speed and direction </text--->"
    "    <text>query move(mp,reg)</text---> ) )";

#ifdef MRA_TRAVERSED
static const string traversedspec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "  ( <text>mregion -> region</text--->"
    "    <text>traversed( _ )</text--->"
    "    <text>Projection of a moving region into "
    "the plane.</text--->"
    "    <text>traversed(mregion1)</text---> ) )";
#endif // MRA_TRAVERSED

#ifdef MRA_PREC
static const string mraprecspec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "  ( <text>(real) -> bool</text--->"
    "    <text>mraprec( _ , _ )</text--->"
    "    <text>Sets precision of comparisons. Two floating point values "
    "are considered equal by the algebra, if their difference is lower "
    "or equal than the first parameter. The second parameter specifies "
    "a relaxed comparison precision for the check whether a moving "
    "segment is collinear with itself in its the initial and final "
    "instant. In this case, two floating point values are considered "
    "equal if their difference is lower or equal than the first "
    "parameter times the second parameter. The operator "
    "always returns true.</text--->"
    "    <text>mraprec(0.0001, 10)</text---> ) )";
#endif // MRA_PREC

static const string bboxspec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "  ( <text>(uregion) -> rect3</text--->"
    "    <text>bbox( _ )</text--->"
    "    <text>Returns the 3d bounding box of the unit.</text--->"
    "    <text>bbox(mregion1)</text---> ) )";

static const string verttrajectoryspec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "  ( <text>(uregion || mregion) -> line</text--->"
    "    <text>vertextrajectory( _ )</text--->"
    "    <text>Computes the trajectory of the vertices </text--->"
    "    <text>vertextrajectory(mregion1)</text---> ) )";

const string mraunitsspec  =
  "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
  "  ( <text>movingregion -> (stream uregion)</text--->"
  "<text>units( _ )</text--->"
  "<text>Create the stream of all uregions contained by "
  "the moving region.</text--->"
  "<text>units( mregion1 )</text--->) )";

/*
Used for unit testing only.

*/

#ifdef MRA_UNITTEST
static const string unittestspec =
    "( ( \"Signature\" \"Syntax\" \"Meaning\" \"Example\" ) "
    "  ( <text>unit testing only</text--->"
    "    <text>unit testing only</text--->"
    "    <text>unit testing only</text--->"
    "    <text>unit testing only</text---> ) )";
#endif // MRA_UNITTEST

/*
1.1 Operator creation

*/

static Operator atinstant("atinstant",
                          atinstantspec,
                          2,
                          atinstantvaluemap,
                          AtInstantSelect,
                          AtInstantTypeMap);

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

#ifdef MRA_TRAVERSED
static Operator traversed("traversed",
                          traversedspec,
                          1,
                          traversedvaluemap,
                          MRegionSelect,
                          MRegionToRegionTypeMap);
#endif // MRA_TRAVERSED

#ifdef MRA_PREC
static Operator mraprec("mraprec",
                        mraprecspec,
                        MraprecValueMap,
                        simpleSelect,
                        MraprecTypeMap);
#endif // MRA_PREC

static Operator bbox("bbox",
                     bboxspec,
                     BboxValueMap,
                     simpleSelect,
                     BboxTypeMap);

static Operator move("move",
                     movespec,
                     MoveValueMap,
                     simpleSelect,
                     MoveTypeMap);

static Operator vertextrajectory("vertextrajectory",
                   verttrajectoryspec,
                   2,
                   verttrajectoryvaluemap,
                   VertTrajectorySelect,
                   VertTrajectoryTypeMap);

static Operator mraunits("units",
                         mraunitsspec,
                         mraunitsvalmap,
                         Operator::SimpleSelect,
                         MRAUnitsTypeMap);


/*
Used for unit testing only.
  
*/

#ifdef MRA_UNITTEST
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
static Operator unittest3("unittest3",
                          unittestspec,
                          Unittest3ValueMap,
                          simpleSelect,
                          Unittest3TypeMap);
#endif // MRA_UNITTEST

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
    AddOperator(&at);
    AddOperator(&bbox);
    AddOperator(&move);
    AddOperator(&vertextrajectory);
    AddOperator(&mraunits);

#ifdef MRA_TRAVERSED
    AddOperator(&traversed);
#endif // MRA_TRAVERSED
    
#ifdef MRA_PREC
    AddOperator(&mraprec);
#endif // MRA_PREC



/*    
 Used for unit testing only.
  
*/
#ifdef MRA_UNITTEST
    AddOperator(&unittest1);
    AddOperator(&unittest2);
    AddOperator(&unittest3);
#endif // MRA_UNITTEST
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
