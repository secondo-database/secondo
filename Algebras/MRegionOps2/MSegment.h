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

[1] File Refinement2.h

This file contains classes and functions for use within the operators intersection and inside

[TOC]

1 Helper classes and methods

1.1 Some forward declaration of helper methods

*/

#ifndef MSEGMENT_H_
#define MSEGMENT_H_

#include <gmp.h>
#include <gmpxx.h>
#include "MovingRegionAlgebra.h"
#include "TemporalAlgebra.h"
#include "NList.h"
#include "NumericUtil.h"
#include "PointVector.h"
#include "Segment.h"
#include "Statistic.h"
#include "DateTime.h"
#include "StopWatch.h"
#include <vector>
#include <set>
#include <list>


namespace mregionops2 {

class PFace;
class Segment3D;

/*
1 Class MSegment

This class represents a moving segment, which is constructed by
the ~ResultUnitFactory~.

*/

class MSegment {

public:

/*
1.1 Constructor

*/

    MSegment(const Segment3D& initial,
             const Segment3D& median,
             const Segment3D& final,
             const PFace* const pFace);

/*
1.1 Getter and setter methods

1.1.1 GetPFace

Returns a pointer to the ~PFace~, this ~MSegment~ comes from.

*/

    inline const PFace* GetPFace() const {

        return pFace;
    }

/*
1.1.1 GetInitial/GetFinal

Returns the initial/final state of this ~MSegment~.
This segment might be degenerated to a point.

*/

    inline const Segment2D& GetInitial() const {

        return initial;
    }

    inline const Segment2D& GetFinal() const {

        return final;
    }

/*
1.1.1 GetMedian

Returns the median state of this ~MSegment~.
This segment can never degenerate.

*/

    inline const Segment2D& GetMedian() const {

        return median;
    }

/*
1.1.1 GetMedianHS

Returns the median state of this ~MSegment~ as ~HalfSegment~.
This segment can never degenerate.

*/

    inline const HalfSegment& GetMedianHS() const {

        return medianHS;
    }

/*
1.1.1 GetFaceNo/GetCycleNo/GetSegmentNo

Returns the faceno/cycleno/edgeno of this ~MSegment~.

*/

    inline int GetFaceNo() const {

        return medianHS.attr.faceno;
    }

    inline int GetCycleNo() const {

        return medianHS.attr.cycleno;
    }

    inline int GetSegmentNo() const {

        return medianHS.attr.edgeno;
    }

/*
1.1.1 GetInsideAbove

Returns the flag insideAbove of this ~MSegment~.

*/

    inline int GetInsideAbove() const {

        return insideAbove;
    }


/*
1.1.1 IsLeftDomPoint

Returns ~true~, if this ~MSegment~ is a left one.
Note: This is indicated by the median ~HalfSegment~.

*/

    inline bool IsLeftDomPoint() const {

        return medianHS.IsLeftDomPoint();
    }

/*
1.1.1 SetLeftDomPoint

Marks this ~MSegment~ as left.

*/

    inline void SetLeftDomPoint(bool ldp) {

        medianHS.SetLeftDomPoint(ldp);
    }

/*
1.1.1 SetSegmentNo

Sets the segmentno to sn.

*/

    inline void SetSegmentNo(int sn) {

        medianHS.attr.edgeno = sn;
    }

/*
1.1 Operators and Predicates

1.1.1 CopyIndicesFrom

Copies the faceno/cycleno/edgeno from hs to this.

*/

    inline void CopyIndicesFrom(const HalfSegment* hs) {

        medianHS.attr.faceno = hs->attr.faceno;
        medianHS.attr.cycleno = hs->attr.cycleno;
        medianHS.attr.edgeno = hs->attr.edgeno;
        //medianHS.attr.insideAbove = hs->attr.insideAbove;
    }

/*
1.1.1 IsParallel

Returns ~true~, if this ~MSegment~ is parallel to ms.

*/

    bool IsParallel(const MSegment& ms) const;

/*
1.1.1 LessByMedianHS

Returns ~true~, if the median ~HalfSegment~ of this is
lower than the median ~HalfSegment~ of ms,
according to the ~HalfSegment~ order, specified in the ~SpatialAlgebra~.

*/

    inline bool LessByMedianHS(const MSegment& ms) const {

        return GetMedianHS() < ms.GetMedianHS();
    }

/*
1.1.1 LogicLess

Returns ~true~, if the median ~HalfSegment~ of this is
lower than the median ~HalfSegment~ of ms,
similar to ~HalfSegment::LogicCompare~, specified in the ~SpatialAlgebra~.

*/

    inline bool LogicLess(const MSegment& ms) const {

        if (IsLeftDomPoint() != ms.IsLeftDomPoint())
            return IsLeftDomPoint() > ms.IsLeftDomPoint();

        return GetMedianHS().LogicCompare(ms.GetMedianHS()) == -1;
    }

/*
1.1 Methods for debugging

*/

    void Print() const;

private:

/*
1.1 Attributes

1.1.1 initial/median/final

The initial/median/final state as ~Segment2D~.

*/

    Segment2D initial;
    Segment2D median;
    Segment2D final;

/*
1.1.1 medianHS

The median state as ~HalfSegment~.

*/

    HalfSegment medianHS;

/*
1.1.1 insideAbove

The flag insideAbove.

*/

    bool insideAbove;

/*
1.1.1 pFace

A pointer to the ~PFace~, this ~MSegment~ comes from.

*/

    const PFace* pFace;
};

} // end namespace

#endif /*MSEGMENT_H_*/
