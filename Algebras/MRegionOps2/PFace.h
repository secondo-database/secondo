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


[TOC]

1 Helper classes and methods

1.1 Some forward declaration of helper methods

*/


/*

1 class deklarations

class PFace

2 Defines and Includes

*/

#ifndef PFace_H_
#define PFace_H_
#include "MovingRegion2Algebra.h"
#include "PointVector.h"
#include "Segment.h"
#include "Refinement3.h"
#include "IntersectionSegment.h"
#include "IntSegContainer.h"
#include "Point3DExt.h"
#include "ResultUnitFactory.h"
#include "SetOps2.h"

namespace mregionops2 {

class PFace;


/*

1 Class PFace

This class represents essentially a movingsegment,
viewed as a polygonal face in a 3-dimensional xyt-space(time).
Due to the restrictions of movingsegments,
a ~PFace~ can be either a planar trapezium or a triangle.

To simplify computations, we define another 2-dimensional coordinate system for
each ~PFace~: The wt-system, which is basically constructed as follows:

  * We keep the t-axis from the xyt-system.

  * The w-unitvector is given by the crossproduct of the t-unitvector and
    the normalvector of the ~PFace~.

*/

class PFace {

public:

/*

1.1 Constructor
using four points vs MSegmentData in previous try

*/

PFace(SourceUnit2* _unit, Point2D _is, Point2D _ie, Point2D _fs, 
      Point2D _fe, bool _insideAbove);
void Print();

/*

1.1 Getter and setter methods

1.1.1 Intersection

Computes the intersection of this ~PFace~ with pf and adds
one ~IntersectionSegment~ instance to both ~PFaces~.

*/

    void Intersection(PFace& pf);

/*

1.1.1 GetMSeg

Returns a pointer to the corresponding ~MSegmentData~ instance.

*/

//    inline const MSegmentData* GetMSeg() const {
//        return mSeg;
//    }

/*
1.1.1 GetNormalVector

Returns the normal unitvector of this ~PFace~,
which is pointing to the outside.

*/

    inline Vector3D GetNormalVector() const {

        return normalVector;
    }

/*
1.1.1 GetOperation

Returns an enum to indicate the current set operation:
INTERSECTION, UNION or MINUS.

*/

    inline const SetOp GetOperation() const {

        return unit->GetOperation();
    }

/*
1.1.1 touches

Returns ~true~, if this ~PFace~ is parallel to the ~PFace~ pf.

*/

    bool touches(const PFace& pf) const;


/*
1.1.1 IsParallelTo

Returns ~true~, if this ~PFace~ is parallel to the ~PFace~ pf.

*/

    bool IsParallelTo(const PFace& pf) const;

/*
1.1.1 IsCoplanarTo

Returns ~true~, if this ~PFace~ is coplanar to the argument.

*/

    bool IsCoplanarTo(const PFace& pf) const;
    bool IsCoplanarTo(const Point3D& p) const;
    bool IsCoplanarTo(const Segment3D& s) const;
    bool IsCoplanarTo(const IntersectionSegment& s) const;

/*
1.1.1 IsPartOfUnitA/B

Returns ~true~, if this ~PFace~ belongs to ~SourceUnit~ A/B.

*/

    inline bool IsPartOfUnitA() const {

        return unit->IsUnitA();
    }

    inline bool IsPartOfUnitB() const {

        return !unit->IsUnitA();
    }



/*
1.1 Operators and Predicates

1.1.1 AEqualsB

Returns ~true~, if this ~PFace~ is a triangle and
the edge AB is degenerated to a point.

*/

    inline bool AEqualsB() const {

        //return mSeg->GetPointInitial(); ??
          return true;
    }

/*
1.1.1 GetStartTime/GetEndTime

Returns the min./max. value of t.


*/
    inline mpq_class GetStartTime() const {

        return unit->GetStartTime();
    }

    inline mpq_class GetEndTime() const {

        return unit->GetEndTime();
    }


/*
1.1.1 GetA XYT

Returns the lower left point in xyt-coords of the ~PFace~,
if viewed from outside.

*/

    inline Point3D GetA_XYT() const {

        if (InitialStartPointIsA())
            return Point3D(is.GetX(), is.GetY(),
                    unit->GetStartTime());
        else
            return Point3D(ie.GetX(), ie.GetY(),
                    unit->GetStartTime());
    }

/*
1.1.1 GetB XYT

Returns the lower right point in xyt-coords of the ~PFace~,
if viewed from outside.

*/

    inline Point3D GetB_XYT() const {

        if (InitialStartPointIsA())
            return Point3D(ie.GetX(), ie.GetY(),
                    unit->GetStartTime());
        else
           return Point3D(is.GetX(), is.GetY(),
                    unit->GetStartTime());
    }

/*
1.1.1 GetC XYT

Returns the upper left point in xyt-coords of the ~PFace~,
if viewed from outside.

*/

    inline Point3D GetC_XYT() const {

        if (InitialStartPointIsA())
            return Point3D(fs.GetX(), fs.GetY(),
                    unit->GetEndTime());
        else
            return Point3D(fe.GetX(), fe.GetY(),
                    unit->GetEndTime());
    }

/*
1.1.1 GetD XYT

Returns the upper right point in xyt-coords of the ~PFace~,
if viewed from outside.

*/

    inline Point3D GetD_XYT() const {

        if (InitialStartPointIsA())
            return Point3D(fe.GetX(), fe.GetY(),
                    unit->GetEndTime());
        else
            return Point3D(fs.GetX(), fs.GetY(),
                    unit->GetEndTime());
    }

/*
1.1 Methods used by ~ResultUnitFactory~.

The ~ResultUnitFactory~ performs a plane-sweep in t-direction
from bottom to top, over all relevant ~PFaces~, to construct the result units.

1.1.1 Finalize

This method is called before the plane-sweep starts and
adds two ~IntersectionSegments~ to each relevant ~PFace~:
The left and right border.


*/
    inline void Finalize() {

        DecideRelevanceAndAddBoundary();

    }

/*
1.1.1 GetMidPoint

Returns the midpoint of this ~PFace~.
Note that this point is never part of the boundary.

*/

    const Point3D GetMidPoint() const;

/*
1.1.1 GetState

Returns an enum to indicate the state of this ~PFace~.

*/

    inline State GetState() const {

        return state;
    }

/*
1.1.1 MarkAsEntirelyOutside
Marks this ~PFace~ as entirely outside the other mregion.

*/

    inline void MarkAsEntirelyOutside() {
        state = ENTIRELY_OUTSIDE;
    }

/*
1.1.1 IsEntirelyOutside
Returns ~true~, if this ~PFace~ is entirely outside the other mregion.

*/

    inline bool IsEntirelyOutside() const {
        return state == ENTIRELY_OUTSIDE;
    }

/*
1.1.1 MarkAsEntirelyInside
Marks this ~PFace~ as entirely inside the other mregion.

*/

    inline void MarkAsEntirelyInside() {
        state = ENTIRELY_INSIDE;
    }

/*
1.1.1 IsEntirelyInside
Returns ~true~, if this ~PFace~ is entirely inside the other mregion.

*/

    inline bool IsEntirelyInside() const {
        return state == ENTIRELY_INSIDE;
    }

/*
1.1.1 MarkAsNormalRelevant

Marks this ~PFace~ as normal relevant for the result mregion.
This means, that no critical case occured during the intersection process.

*/

    inline void MarkAsNormalRelevant() {

        if (!IsCriticalRelevant())
            state = RELEVANT_NOT_CRITICAL;
    }

/*
1.1.1 MarkAsCriticalRelevant
Marks this ~PFace~ as critical relevant for the result mregion.
This means, that at least one critical case occured during the
intersection process.

*/

    inline void MarkAsCriticalRelevant() {
        state = RELEVANT_CRITICAL;
    }

/*
1.1.1 MarkAsIrrelevant
Marks this ~PFace~ as irrelevant for the result mregion.

*/

    inline void MarkAsIrrelevant() {
        state = NOT_RELEVANT;
    }

/*
1.1.1 IsNormalRelevant
Returns ~true~, if this ~PFace~ is normal relevant for the result mregion.

*/

    inline bool IsNormalRelevant() const {
        return state == RELEVANT_NOT_CRITICAL;
    }

/*
1.1.1 IsCriticalRelevant
Returns ~true~, if this ~PFace~ is critical relevant
for the result mregion.

*/

    inline bool IsCriticalRelevant() const {
        return state == RELEVANT_CRITICAL;
    }

/*
1.1.1 IsIrrelevant
Returns ~true~, if this ~PFace~ is irrelevant for the result mregion.

*/

    inline bool IsIrrelevant() const {
        return state == NOT_RELEVANT;
    }

private:

/*
1.1 Private methods

*/

   inline bool InitialStartPointIsA() const {

        return initialStartPointIsA;
   }

    void SetInitialStartPointIsA(bool insideAbove);
    void ComputeBoundingRect();
    void ComputeNormalVector();
    void ComputeWTCoords();
    bool IntersectionPlaneSegment(const Segment3D& seg, Point3D& result);
    void DecideRelevanceAndAddBoundary();
    void AddEntireBoundary();
    void AddIntSeg(const Segment3D& seg, PFace& other);
    void AddBoundaryIntSeg(const Segment3D& seg);

/*
1.1.1 TransformToWT

Transforms the argument from the xyt-system to the wt-system.

*/

    inline mpq_class TransformToW(const Point3D& xyt) const {

        return xyt.GetX() * GetWVector().GetX() +
               xyt.GetY() * GetWVector().GetY();
    }

/*
1.1.1 GetWVector

Returns the w-unitvector of this ~PFace~,
which defines the w-axis of the wt-system.

The vector w is either the normalized cross product
of the normal vector and the t-unit-vector, or it's negative.
This depends on the kind of set-operation, we want to perform.

*/

    inline Vector3D GetWVector() const {
        return wVector;
    }

/*
1.1 Attributes

1.1.1 unit

A pointer to the ~SourceUnit~, this ~PFace~ belongs to.

*/

    SourceUnit2* const unit;

/*
1.1.1 boundingRect

The projection bounding rectangle in xy-coords.

*/

    Rectangle<2> boundingRect;

/*
1.1.1 normalVector

The normal vector of this ~PFace~, pointing to the outside.

*/

    Vector3D normalVector;

/*
1.1.1 intSegs

A container to store all ~IntersectionSegments~ of this ~PFace~.

*/

    IntSegContainer intSegs;

/*
1.1.1 wVector

The w-unitvector of this ~PFace~,
which defines the w-axis of the wt-system.

The vector w is either the normalized cross product
of the normal vector and the t-unit-vector, or it's negative.
This depends on the kind of set-operation, we want to perform.

*/

// implementation create intersection segments

/*

1.1.1 aW, bW, cW, dW

The w-coords of the four vertices of this ~PFace~.

*/

    mpq_class aW;
    mpq_class bW;
    mpq_class cW;
    mpq_class dW;

/*
1.1.1 state

An enum, indicating the state of this ~PFace~. Possible values are:

  * $UNKNOWN$
  * $ENTIRELY\_INSIDE$
  * $ENTIRELY\_OUTSIDE$
  * $RELEVANT\_NOT\_CRITICAL$
  * $RELEVANT\_CRITICAL$
  * $NOT\_RELEVANT$

*/

    State state;

/*
1.1.1 initialStartPointIsA

A flag, indicating if the vertex A equals the ~initialStartPoint~ of the
corresponding ~MSegmentData~ instance.

*/

    bool initialStartPointIsA;

    Vector3D wVector;

/*

1.1.1 points

*/
 	Point2D is;
 	Point2D ie;
 	Point2D fs;
 	Point2D fe;
};
}
#endif /*PFace_H_*/

