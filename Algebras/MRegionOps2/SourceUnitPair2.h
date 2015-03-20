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

[1] Implementation

Oktober 2014 - Maerz 2015, S. Schroeer for master thesis.

[TOC]

1 Introduction

This class is essentially a container for both ~SourceUnits~ A and B

2 Defines and Includes

*/


#ifndef SOURCEUNITPAIR2_H_
#define SOURCEUNITPAIR2_H_

#include <gmp.h>
#include <gmpxx.h>

#include "SetOps2.h"
#include "SourceUnit2.h"
#include "Refinement3.h"
#include "ResultUnitFactory.h"

namespace mregionops2 {

class SourceUnitPair2 {

public:

/*

1.1 Constructor

The constructor takes essentially two pointers to ~URegionEmb2~ instances,
one pointer to the global result ~MRegion2~ and an enum to indicate the
kind of set operation.

*/

    SourceUnitPair2(MRegion2* const _unitA,
                    int _aPos, 
                    precTimeInterval _interval,
                    MRegion2* const _unitB,
                    int _bPos,
                    const SetOp _operation,
                    MRegion2* const resultMRegion);

    SourceUnit2 unitA;
    int aPos;
    precTimeInterval interval;
    SourceUnit2 unitB;
    int bPos;
    const SetOp op;
    MRegion2* const resultMRegion;
//    bool* const resultBoolean;
    Rectangle<2> overlapRect;  

/*

1.1 Operators

1.1.1 Operate

This method runs the main-loop of the set operation by calling
the following methods:

*/

    void Operate();

/*
1.1 Getter and setter methods

1.1.1 GetOperation

Returns an enum to indicate the current set operation:
INTERSECTION, UNION or MINUS.

*/

    inline const SetOp GetOperation() const
    {
        return op;
    }

/*
1.1.1 GetOverlapRect

Returns the intersection of the projection bounding rectangles of
both ~SourceUnits~, A and B.

*/

    inline Rectangle<2> GetOverlapRect() const {
        return overlapRect;
    }
/*
1.1.1 HasOverlappingBoundingRect

Returns ~true~, if the intersection of the projection bounding rectangles of
both ~SourceUnits~ is not empty.

*/

    inline bool HasOverlappingBoundingRect() const {
     return overlapRect.IsDefined();
    
    }

    inline void AddTimestamp(mpq_class tim) {  
        timestamps.insert(tim);
    }

    inline unsigned int GetTimeIntervalCount() {
      return timestampVector.size() - 1;
    }

    inline mpq_class GetTimeIntervalStart(unsigned int i) {
      return timestampVector.at(i);
    }

    inline mpq_class GetTimeIntervalEnd(unsigned int i) {
      return timestampVector.at(i + 1);
    }
    inline bool GetSpecialOperationsResult() {
      return specialOperationsResult;
    }
   
    
private:

//  set with t-values from Schnittsegmentendpunkte 
//  sorted list
    set<mpq_class> timestamps;
//  faster access, because direct access of i-tes element is possible
//  the i-te element vs. SET structure
//  in SetTimestampVector the vector with elements are described
    vector<mpq_class> timestampVector;

    void SetTimestampVector();
/*

1.1 Methods for debugging

*/

    void PrintPFaces();
    void ToVrmlFile(bool a, bool b, bool res);

/*
1.1 Private methods

1.1.1 CreatePFaces

*/
    void CreatePFaces();

/*

1.1.1 ComputeIntSegs

Intersects each ~PFace~ from ~SourceUnit A~ with each ~PFace~ from
~SourceUnit B~. The result might be an ~IntersectionSegment~ pair.

*/

    void ComputeIntSegs();

/*

1.1.1 CollectRelevantPFaces

Performs a linear scan over all ~PFaces~ of both ~SourceUnits~,
collecting those which are relevant for the result and
pass them to the ~ResultUnitFactory~.

*/

//    void CollectRelevantPFaces();

/*
1.1.1 ConstructResultUnits

Runs the construction of all result units by calling
~ResultUnitFactory::Start~.

*/

 //   void ConstructResultUnits();

/*
1.1.1 IsEntirelyOutside

Returns ~true~, if pFace is entirely outside of this ~SourceUnit~.

Precondition: pFace is already known as entirely inside or entirely outside!

*/

    bool IsEntirelyOutside(const PFace* pFace);

/*
1.1.1 ComputeOverlapRect

Computes the intersection of the projection bounding rectangles of
both ~SourceUnits~.

*/

    void ComputeOverlapRect();

 ResultUnitFactory resultUnitFactory;

 vector<PFace*> myRelevantPFaces;

 ResultUnit* resultUnit;
 void ConstructResultUnitAsURegionEmb();
 void BuildNewResultUnit(double start, double end,unsigned int index);
// for debugging in ConstructResultUnitAsURegionEmb()
    unsigned int noUnits;
    unsigned int noEmptyUnits;
    bool specialOperationsResult;
};

}
#endif
