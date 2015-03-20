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

[1] Implementation

Oktober 2014 - Maerz 2015, S. Schroeer for master thesis.

[TOC]

1 Helper classes and methods

1.1 Some forward declaration of helper methods

*/

#ifndef INTERSECTIONSEGMENT_H_
#define INTERSECTIONSEGMENT_H_

#include <gmp.h>
#include <gmpxx.h>
#include <assert.h>

#include "SetOps2.h"
#include "Segment3D.h"
#include "Angle.h"


using namespace std;

namespace mregionops2 {

class PFace;

/*
1 Class IntersectionSegment

This class represents essentially an oriented segment in the xyt-space.
Each intersection of two non-disjunct ~PFaces~ create a pair of two
~IntersectionSegment~ instances.
Each instance keeps a pointer to the ~PFace~, it belongs to.

*/

class IntersectionSegment {

public:

/*
1.1 Constructor

The constructor assures the condition: $startpoint.t <= endpoint.t$

*/

      IntersectionSegment(const Segment3D& s, PFace* pFace, Angle leftAng,
                        Angle rightAng, Direction dir);

/*
1.1 Getter and setter methods

1.1.1 GetPFace

Returns a pointer to the ~PFace~, this instance belongs to.

*/

    inline PFace* GetPFace() const {

        assert(pFace != 0);
        return pFace;
    }


/*
1.1.1 SetWCoords

Intitialise the fields ~startWT~ and ~endWT~ by the transformations:

  * startXYT [->] startWT
  * endXYT [->] endWT

*/

    void SetWCoords();

/*
1.1.1 Get$<$Coordinate$>$ methods

This methods returns points of this ~IntersectionSegment~
either in xyt-coords or in wt-coords.

*/


    inline const Point3D* GetStartXYT() const {
        return &startXYT;
    }

    inline const Point3D* GetEndXYT() const {
        return &endXYT;
    }



    inline const Point2D GetStartXY() const {
        return Point2D(startXYT);
    }

    inline const Point2D GetEndXY() const {
        return Point2D(endXYT);
    }


    inline mpq_class GetStartT() const {
        return startXYT.GetT();
    }

    inline mpq_class GetEndT() const {
        return endXYT.GetT();
    }

    inline const Point2D* GetStartWT() const {
        return &startWT;
    }

    inline const Point2D* GetEndWT() const {
        return &endWT;
    }

    inline mpq_class GetStartW() const {
        return startWT.GetW();
    }

    inline mpq_class GetEndW() const {
        return endWT.GetW();
    }

    inline mpq_class GetIntervalStartW() const {
        return intervalStartW;
    }

    inline mpq_class GetIntervalEndW() const {
        return intervalStartW;
    }

    AreaDirection GetAreaDirection();

/*
1.1 Operators and Predicates

1.1.1 IsOrthogonalToTAxis

Returns ~true~, if this is parallel to the xy-plane.

*/

    inline bool IsOrthogonalToTAxis() const {
        return (GetStartT() == GetEndT());
    }

    void Finalize();
    void UpdateWith(IntersectionSegment* seg);
    void InvertAreaDirection();
    bool IsLeftOf(const IntersectionSegment* intSeg) const;
    void ComputeIntervalW(const mpq_class s, const mpq_class e);

/*
1.1.1 Evaluate

Returns the point (in xyt-coords) on this segment with t-coord t.
Precondition: $startpoint.t <= t <= endpoint.t$

*/

    Point3D Evaluate(const mpq_class t) const;

    void Print();

private:

/*
1.1 Private methods

*/

    inline void SetStartWT(const Point2D& _startWT) {
        startWT = _startWT;
    }

    inline void SetEndWT(const Point2D& _endWT) {
        endWT = _endWT;
    }

/*
1.1 Attributes


1.1.1 startXYT and endXYT

Start- and endpoint of this segment in xyt-coordinates.
Note that they are independed of the ~PFace~.

*/

    Point3D startXYT;
    Point3D endXYT;

/*
1.1.1 startWT and endWT

Start- and endpoint of this segment in wt-coordinates.
Note that the w-coord depends on the ~PFace~.
We store them here for the sake of efficency.

*/

    Point2D startWT;
    Point2D endWT;

    Angle rightNeighbour;
    Direction rightNeighbourDir;

    Angle leftNeighbour;
    Direction leftNeighbourDir;
        
    mpq_class intervalStartW;
    mpq_class intervalEndW;

    AreaDirection areaDirection;

/*
1.1.1 pFace

A pointer to the ~PFace~, this instance belongs to.

*/

    PFace* pFace;
};



} // namespace end

#endif /*INTERSECTIONSEGMENT_H_*/
