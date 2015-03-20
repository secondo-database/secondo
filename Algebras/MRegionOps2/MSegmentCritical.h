/*
----
This file is part of SECONDO.

Copyright (C) 2008, University in Hagen, Department of Computer Science,
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
//[x] [$\times $]
//[->] [$\rightarrow $]
//[pow] [\verb+^+]

[1] Headerfile of the Point and Vector classes

April - November 2008, M. H[oe]ger for bachelor thesis.

[2] Implementation with exakt dataype

Oktober 2014 - Maerz 2015, S. Schroeer for master thesis.

[TOC]

1 Introduction

This file contains the definitions of the classes Point2D

2 Defines and Includes

*/

/*
1 Class MSegmentCritical

This class extends the class ~MSegment~ with the attribute ~midPoint~,
which is needed to handle critical ~MSegments~.
A pair of critical ~MSegments~ is constructed as a result of the
intersection of two overlapping coplanar PFaces.

*/

class MSegmentCritical : public MSegment {

public:

/*
1.1 Constructor

*/

    MSegmentCritical(const Segment3D& _initial,
                     const Segment3D& _median,
                     const Segment3D& _final,
                     const Point3D& _midPoint,
                     const PFace* const _pFace) :

      MSegment(_initial, _median, _final, _pFace),
      midPoint(_midPoint) {

    }

/*
1.1 Getter and setter methods

1.1.1 GetMidPoint

Returns the midpoint of this ~MSegmentCritical~, which is equal to
the midpoint of the median ~HalfSegment~.

*/

    inline const Point2D& GetMidpoint() const {
        return midPoint;
    }

/*
1.1 Operators and Predicates

1.1.1 IsPartOfUnitA

Returns ~true~, if the ~Face~ of this belongs to ~SourceUnit~ A.

*/

    bool IsPartOfUnitA() const;

/*
1.1.1 HasEqualNormalVector

Returns ~true~, if the normal vectors of this' and pf' ~Face~ are equal.

*/

    bool HasEqualNormalVector(const MSegmentCritical& msc) const;

/*
1.1.1 Operator $<$

Returns ~true~, if the midPoint of this is lower as the midpoint of msc.

*/

    bool operator <(const MSegmentCritical& msc) const {
        return midPoint < msc.midPoint;
    }

private:

/*
1.1 Attributes

1.1.1 midPoint

The midpoint of this ~MSegmentCritical~, which is equal to
the midpoint of the median ~HalfSegment~.

*/

    Point2D midPoint;
};
