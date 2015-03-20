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


#ifndef SOURCEUNIT2_H_
#define SOURCEUNIT2_H_
#include <gmp.h>
#include "Refinement3.h"
#include "MovingRegion2Algebra.h"
#include "SetOps2.h"

namespace mregionops2 {

class PFace;
class SourceUnitPair2;


struct CycleInfo { 
         PFace* plumblineCandidate; 
         CycleStatus status;  
       };

class SourceUnit2 {

    friend class SourceUnitPair2;

public:

/*

1.1 Constructor

Note that the interval of the parameter ~uRegion~ might be changed!

*/

    SourceUnit2(const bool _isUnitA,
               MRegion2* const _mRegion,
               const int _pos,
               precTimeInterval _interval,
               SourceUnitPair2* const _parent);

/*

1.1 Destructor

*/
  
  ~SourceUnit2();

/*
1.1 Getter and setter methods

1.1.1 IsEmpty

Returns ~true~, if this ~SourceUnit~ represents an empty unit.

*/

    inline bool IsEmpty() const {
        return (pos == -1);
    }

    inline mpq_class GetStartTime() {
       return interval.start;
    }

    inline mpq_class GetEndTime() {
       return interval.end;
    }
    
    inline precTimeInterval GetTimeInterval() {
      return interval;
    }

    const SetOp GetOperation() const;

    void FinalizeIntSegs();


    inline bool IsUnitA() const {
        return isUnitA;
    }

    inline bool IsUnitB() const {
        return !isUnitA;
    }
    
    void SetCycleStatus(unsigned int pfaceNo, 
                        unsigned int cycleNo,  
                        CycleStatus status);    
    void AddTimestamp(mpq_class tim);
    unsigned int GetTimeIntervalCount();
    mpq_class GetTimeIntervalStart(unsigned int i);
    mpq_class GetTimeIntervalEnd(unsigned int i);   

   SourceUnit2* GetPartner();
   inline void SetPartner(SourceUnit2* p)
   {
	partner = p;
   }
   vector<PFace*> GetPFaces(); 
   int GetPFaceCount(); 
   void AddToMRegion(MRegion2* const target);

   bool HasIntersecs();
   bool IsInsidePartner();

private:
    void NormalizeTimeInterval();
    void ComputeBoundingRect();
    const Region GetTestRegion(const double t);
    unsigned int myDebugId;

/*
1.1 Attributes

1.1.1 isUnitA

A flag, indicating which argument of the set operation
this ~SourceUnit~ represents: A or B.

*/

    const bool isUnitA;

/*
1.1.1 isEmpty

A flag, indicating that this ~SourceUnit~ is empty.

*/

    const int pos;

/*

1.1.1 uRegion

A pointer to the corresponding ~URegionEmb~.

Note that the interval of uRegion might be changed!

*/

    MRegion2* const mRegion;
    precTimeInterval interval;
/*

1.1.1 array

A pointer to the corresponding ~DBArray<MSegmentData>~.
1.1.1 partner

A pointer to  the other ~SourceUnit~ instance, which
is the other argument of the set operation.

*/

    SourceUnit2* partner;

/*

1.1.1 parent
A pointer the parent object.

*/

    SourceUnitPair2* const parent;

/*

1.1.1 pFaces
A ~std::vector~ to store all ~PFaces~.

*/

    vector<PFace*> pFaces;
    map<pair<unsigned int, unsigned int>, CycleInfo*> cycleInfo;


/*

1.1 Operators and Predicates
1.1.1 CreatePFaces

Creates a new ~PFace~ for each ~MSegmentData~ of this unit.

*/

    void CreatePFaces();

/*
1.1.1 CollectRelevantPFaces

Performs a linear scan over the list of all ~PFaces~,
collecting those which are relevant for the result and
pass them to the ~ResultUnitFactory~.

*/

	void CollectRelevantPFaces(vector<PFace*>* storage);
	bool IsPFaceInsidePartner(PFace* pface); 
/*

1.1 Methods for debugging and internal helper tasks

*/

    void PrintPFaces();
    bool pointInPolygon(float  polyX[],
         float  polyY[],float  x, float y, int polyCorners);
/*

1.1.1 pFace

A pointer to the ~PFace~, this instance belongs to.

*/

    const PFace* pFace;


    void restrictSegtoInterval(MSegmentData2 sourceSeg, 
                            precTimeInterval sourceInt, 
                            precTimeInterval destInt, 
                            MSegmentData2* destSeg);


};
}
#endif
