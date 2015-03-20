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

[1] Headerfile of ResultUnitFactory class

April - November 2008, M. H[oe]ger for bachelor thesis.

[2] Implementation with exakt dataype

Oktober 2014 - Maerz 2015, S. Schroeer for master thesis.

[TOC]

1 Introduction

2 Defines and Includes

*/
#ifndef RESULTUNITFACTORY_H_
#define RESULTUNITFACTORY_H_

#include <gmp.h>
#include "MovingRegion2Algebra.h"
//#include "MSegment.h"
#include "Refinement3.h"
#include "IntersectionSegment.h"
#include "IntSegContainer.h"
#include "SetOps2.h"
#include "PFace.h"
using namespace std;
namespace mregionops2 {

class SourceUnitPair2;

/*

1 Class ResultUnitFactory

This class essentially collects all relevant ~PFaces~ from both ~SourceUnits~
and performs afterwards a plane-sweep in t-direction from bottom to top
over all ~PFaces~. For each relevant timevalue a new ~ResultUnit~ is
constructed and added to the result ~MRegion~.

*/

struct DoubleCompare {

    inline bool operator()(const double& d1, const double& d2) const {
        if(d1 < d2)
	{
		return d1;	
	}
	else
		return d2;
        //return NumericUtil::Lower(d1, d2);
    }
};

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

	inline void AddPFace(PFace* pf)
	{ pFaces.push_back(pf);}

    mpq_class t1;
    mpq_class t12;
    mpq_class t2;

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

 //   ResultUnit* resultUnit;

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


    set<double, DoubleCompare> time;
    set<double, DoubleCompare>::const_iterator timeIter;

	vector<PFace*> pFaces;
};


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
	cout << "ResultUnitFactory.h - ResultUnit::Constructor()" << endl;  
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
    
    inline void AddSegment(Segment3D ms) {

        msegments.push_back(ms);
    }
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

    vector<Segment3D> msegments;

/*
1.1.1 index

A counter, used in ~AddSegment~.

*/

    unsigned int index;
};


}

#endif /*RESULTUNITFACTORY_H_*/

