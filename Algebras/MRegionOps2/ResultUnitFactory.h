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


1 class deklarations

class SetOperator2 (startpoint Operate())
class SourceUnitPair2
class SourceUnit2
class ResultUnit
class PFace
class MSegment

2 Defines and Includes

*/

#ifndef RESULTUNITFACTORY_H_
#define RESULTUNITFACTORY_H_

#include "MovingRegion2Algebra.h"
#include "PointVector.h"
#include "Segment.h"
#include "Refinement3.h"
#include "IntersectionSegment.h"
#include "IntSegContainer.h"


namespace mregionops2 {

class SourceUnitPair2;
class SourceUnit2;
class ResultUnitFactory;
class ResultUnit;


/*

1 Class ResultUnitFactory

This class essentially collects all relevant ~PFaces~ from both ~SourceUnits~
and performs afterwards a plane-sweep in t-direction from bottom to top
over all ~PFaces~. For each relevant timevalue a new ~ResultUnit~ is
constructed and added to the result ~MRegion~.

*/

class ResultUnitFactory {

public:

/*

1.1 Constructor

*/

    ResultUnitFactory(MRegion2* const _resMRegion,
                      const SourceUnitPair2* const _parent) :

        resMRegion(_resMRegion),
        parent(_parent),
        noUnits(0),
        noEmptyUnits(0),
        evalutedIntSegs(0),
        noMSegsValid(0),
        noMSegsSkipped(0),
        noMSegCritical(0),
        decisionsByPlumbline(0),
        decisionsByEntirelyInOut(0),
        decisionsByAdjacency(0),
        decisionsByDegeneration(0) {

    }


/*
1.1 Operators and Predicates

1.1.1 Start

This method starts the construction of all ~ResultUnits~.

*/

    void Start();

/*
1.1.1 AddPFace

Adds a relevant ~PFace~.

*/

    inline void AddPFace(PFace* pf) {

        pFaces.push_back(pf);
    }

/*

1.1 Attributes

1.1.1 time, timeIter
A ~std::set~ to store the relevant timevalues and a suitable iterator.

*/

    //set<double, DoubleCompare> time;
    //set<double, DoubleCompare>::const_iterator timeIter;

/*

1.1.1 t1, t12, t2

The start-, middle- and endtime of the current ~ResultUnit~.

*/

    mpq_class t1;
    mpq_class t12;
    mpq_class t2;

/*

1.1.1 pFaces
A ~std::vector~ to store the relevant ~PFaces~.

*/

   vector<PFace*> pFaces;

/*

1.1.1 pFacesReduced

A ~std::vector~ to store a reduced set of ~PFaces~, relevant for the
intersection process. This optimization can be done in advance by using
the projection bounding rectangles of both ~SourceUnits~.

*/

    vector<PFace*> pFacesReduced;

/*

1.1.1 criticalMSegs

A ~std::vector~ to store the critical MSegments.

*/

//    vector<MSegmentCritical> criticalMSegs;

/*
1.1.1 resMRegion

A pointer to the result ~MRegion~.

*/

    MRegion2* resMRegion;

/*
1.1.1 parent

A pointer to the parent object.

*/

    const SourceUnitPair2* const parent;

/*
1.1.1 resultUnit

A pointer to the current ~ResultUnit~.

*/

    ResultUnit* resultUnit;

/*
1.1.1 Attributes used for debugging

*/

    unsigned int noUnits;
    unsigned int noEmptyUnits;
    unsigned int evalutedIntSegs;
    unsigned int noMSegsValid;
    unsigned int noMSegsSkipped;
    unsigned int noMSegCritical;
    unsigned int decisionsByPlumbline;
    unsigned int decisionsByEntirelyInOut;
    unsigned int decisionsByAdjacency;
    unsigned int decisionsByDegeneration;

    vector<string> vrml;

};




/*

1 Class ResultUnit

This class essentially collects at first all ~MSegments~ of a result unit,
produced by the ~ResultUnitFactory~. Afterwards, the ~MSegments~ will be
suitably linked together, to form a proper ~URegionEmb~. This is done by
calling the method EndBulkLoad.

*/


/*
1 Enumeration State

Used in the class ~PFace~ to indicate it's current state.

*/

enum State {

    UNKNOWN,
    ENTIRELY_INSIDE,
    ENTIRELY_OUTSIDE,
    RELEVANT_NOT_CRITICAL,
    RELEVANT_CRITICAL,
    NOT_RELEVANT
};


class ResultUnit {

public:

/*
1.1 Constructor

*/

    ResultUnit(const Interval<Instant> _interval) :
        interval(_interval),
        index(0) {

    }

/*
1.1 Getter and setter methods

1.1.1 GetInterval

Returns the interval as set by the constructor.

*/

    inline const Interval<Instant>& GetInterval() const {

        return interval;
    }

/*
1.1 Operators and Predicates

1.1.1 StartBulkLoad

This method must be called before adding the first ~MSegment~.

*/

    inline void StartBulkLoad() {
        index = 0;
    }


    void EndBulkLoad(bool merge);

/*
1.1.1 ConvertToListExpr

Converts this ~ResultUnit~ to a ~ListExpr~.

*/

    const ListExpr ConvertToListExpr() const;

/*
1.1.1 ConvertToURegionEmb

Converts this ~ResultUnit~ to a ~URegionEmb~.

*/

    URegionEmb* ConvertToURegionEmb(DbArray<MSegmentData>* segments) const;

/*
1.1.1 IsEmpty

Returns ~true~, if this ~ResultUnit~ contains no ~MSegments~.

*/

//    inline bool IsEmpty() const {
//
//        return msegments.size() == 0;
//    }

/*
1.1 Methods for debugging

*/

    void Print(const bool segments = true) const;
    string GetVRMLDesc() const;
    bool Check() const;

private:

/*
1.1 Private methods

1.1 Attributes
1.1.1 interval

The interval of this ~ResultUnit~

*/

    const Interval<Instant> interval;

/*
1.1.1 msegments

The list of the ~MSegments~.

*/

//    vector<MSegment> msegments;

/*
1.1.1 index

A counter, used in ~AddSegment~.

*/

    unsigned int index;
};

}

#endif /*RESULTUNITFACTORY_H_*/

