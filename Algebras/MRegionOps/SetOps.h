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

[1] Headerfile of the Set Operator Classes

April - November 2008, M. H[oe]ger for bachelor thesis.

[TOC]

1 Introduction

This file essentially contains the definitions of several classes which
provides the core functionality of the three set operators
~intersection~, ~union~ and ~minus~ with the signature \\
mregion [x] mregion [->] mregion \\
used in the ~MovingRegionAlgebra~.

A general overview about the relationship of this classes is shown below:

                Figure 1: Class diagram [classdiagram.eps]



2 Defines and Includes

*/

#ifndef SETOPS_H_
#define SETOPS_H_

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

using namespace std;

namespace temporalalgebra{

namespace mregionops {

/*
1.1 $PRINT\_DEBUG\_MESSAGES$

Enables the output of messages useful for debugging.
Note: The output might be very verbose.

*/

//#define PRINT_DEBUG_MESSAGES

/*
1.1 $PRINT\_STATISTIC$

Enables the output of statistic data for each set-operation.

*/

#define PRINT_STATISTIC


#ifdef  PRINT_DEBUG_MESSAGES
#define PRINT_STATISTIC
#endif

/*
1.1 $WRITE\_VRML\_FILE$

Enables the creation of one vrml-file for each source unit pair,
showing it's elements as wireframes in the xyt-space.
Use the flags below to specify the desired output.

Note: The file will be created in the current directory and follows
the naming convention $unitpair\_<starttime>.wrl$

*/

//#define WRITE_VRML_FILE
const bool showSourceUnitA = true;
const bool showSourceUnitB = true;
const bool showResultUnits = true;

/*
1.1 $OPTIMIZE\_BY\_BOUNDING\_RECT$

Enables an optimization, using the
projection bounding rectangles of both source units.
See the class ~SourceUnit~ for details.

*/

#define OPTIMIZE_BY_BOUNDING_RECT

/*
1.1 $CACHE\_TESTREGIONS$

Enables the caching of ~Testregions~, used in the class ~SourceUnit~.
See it's documentation for details.

*/

#define CACHE_TESTREGIONS

/*
1.1 $NORMALIZE\_TIME\_INTERVAL\_OF\_SOURCE\_UNITS$

Enables the normalization of the interval of a ~SourceUnit~ to 1.0.
This might be useful to improve the numerical stability of very thin units.

*/

const bool NORMALIZE_TIME_INTERVAL_OF_SOURCE_UNITS = true;

/*
1.1 $DECIDE\_BY\_ENTIRELY\_IN\_OUT$

Enables an optimization during the construction of a result unit.
See the class ~ResultUnitFactory~ for details.

*/

const bool DECIDE_BY_ENTIRELY_IN_OUT = true;

/*
1.1 $DECIDE\_BY\_PLUMBLINE\_ONLY$

Disables an optimization during the construction of a result unit.
See the class ~ResultUnitFactory~ for details.

*/

const bool DECIDE_BY_PLUMBLINE_ONLY = false;

/*
1.1 $MERGE\_RESULT\_MSEGMENTS$

Merge adjacent and coplanar moving segments of a result unit.

Note: This feature is not implemented yet!

*/

const bool MERGE_RESULT_MSEGMENTS = false;




/*
1 Forward declarations

*/

class SetOperator;
class SourceUnit;
class SourceUnitPair;
class Point3DExt;
struct PointExtCompare;
class PointExtSet;
class IntersectionSegment;
class MSegment;
class MSegmentCritical;
class ResultUnit;
struct DoubleCompare;
class ResultUnitFactory;
struct TestRegion;
struct IntSegCompare;
class IntSegContainer;
class PFace;

/*
1 Enumeration SetOp

Indicates the kind of set operation.

*/

enum SetOp {

    INTERSECTION,
    UNION,
    MINUS
};

/*
1 Class SetOperator

This class provides the three set operations
~intersection~, ~union~ and ~minus~ with the signature
mregion [x] mregion [->] mregion.

*/

class SetOperator {

public:

/*
1.1 Constructor

The constructor takes three pointers to ~MRegion~ instances,
according to the signature a [x] b [->] res.

*/

    SetOperator(MRegion* const _a,
                MRegion* const _b,
                MRegion* const _res) :

                a(_a), b(_b), res(_res) {

    }

/*
1.1 Operators

1.1.1 Intersection

Performs the operation $a \cap b$ and writes the result to res.

*/

    void Intersection();

/*
1.1.1 Union

Performs the operation $a \cup b$ and writes the result to res.

*/

    void Union();

/*
1.1.1 Minus

Performs the operation $a \setminus b$ and writes the result to res.

*/

    void Minus();

private:

/*
1.1 Private methods

*/

    void Operate(const SetOp op);

/*
1.1 Attributes

*/

    MRegion* const a;
    MRegion* const b;
    MRegion* const res;
};

/*
1 Struct TestRegion

This datastructure is used in the class ~SourceUnit~.
See it's documentation for more details.

*/

struct TestRegion {

    inline TestRegion() :

        region(0),
        instant(instanttype),
        defined(false) {

    }

    Region region;
    Instant instant;
    bool defined;
};

/*
1 Class SourceUnit

This class represents a single unit of a ~MRegion~ A or B,
which are the arguments of the set operation.

*/
class SourceUnit {

    friend class SourceUnitPair;

public:

/*
1.1 Constructor

Note that the interval of the parameter ~uRegion~ might be changed!

*/

    SourceUnit(const bool isUnitA,
               URegionEmb* const uRegion,
               const DbArray<MSegmentData>* const array,
               const bool isEmpty,
               SourceUnitPair* const parent);

/*
1.1 Destructor

*/

    ~SourceUnit();

/*
1.1 Getter and setter methods

1.1.1 SetPartner

Sets a pointer the other ~SourceUnit~ instance, which
is the other argument of the set operation.

*/

    inline void SetPartner(SourceUnit* _partner) {

        partner = _partner;
    }

/*
1.1.1 GetPartner

Returns a pointer the other ~SourceUnit~ instance, which
is the other argument of the set operation.

*/

    inline SourceUnit* GetPartner() const {

        return partner;
    }

/*
1.1.1 GetParent

Returns a pointer the parent object.

*/

    inline SourceUnitPair* GetParent() const {

        return parent;
    }

/*
1.1.1 GetParent

Returns a pointer the corresponding ~URegionEmb~ instance.

*/

    inline const URegionEmb* GetURegionEmb() const {

        return uRegion;
    }

/*
1.1.1 IsUnitA/B

Returns ~true~, if this ~SourceUnit~ represents the first/second argument
of the set operation.

*/

    inline bool IsUnitA() const {

        return isUnitA;
    }

    inline bool IsUnitB() const {

        return !isUnitA;
    }

/*
1.1.1 GetOperation

Returns an enum to indicate the current set operation:
INTERSECTION, UNION or MINUS.

*/

    const SetOp GetOperation() const;

/*
1.1.1 IsEmpty

Returns ~true~, if this ~SourceUnit~ represents an empty unit.

*/

    inline bool IsEmpty() const {

        return isEmpty;
    }

/*
1.1.1 HasNormalizedTimeInterval

Returns ~true~, if the interval of this ~SourceUnit~ was normalized to 1.0.
This might be useful to improve the numerical stability of very thin units.

*/

    inline bool HasNormalizedTimeInterval() const {

        return hasNormalizedTimeInterval;
    }

/*
1.1.1 GetTimeInterval

Returns the current interval of this ~SourceUnit~.

*/

    inline const Interval<Instant>& GetTimeInterval() const {

        return uRegion->timeInterval;
    }

/*
1.1.1 GetOriginalTimeInterval

Returns the original interval of this ~SourceUnit~ before normalization.

*/

    inline const Interval<Instant>& GetOriginalTimeInterval() const {

        return originalTimeInterval;
    }

/*
1.1.1 GetStart/EndTime

Returns the start/end of the interval as double value.

*/

    inline double GetStartTime() const {

        return startTime;
    }

    inline double GetEndTime() const {

        return endTime;
    }

/*
1.1.1 GetBoundingRect

Returns the projection bounding rectangle of this ~SourceUnit~ to the xy-plane.

*/

    inline Rectangle<2> GetBoundingRect() const {

        return boundingRect;
    }

/*
1.1.1 GetNoOfPFaces

Returns the number of ~PFaces~ of this ~SourceUnit~,
which is equal to the number of ~MSegmentData~ in the ~URegionEmb~.

*/

    inline unsigned int GetNoOfPFaces() const {

        return pFaces.size();
    }

/*
1.1 Operators and Predicates

1.1.1 AddGlobalTimeValue

Adds a relevant timevalue, which marks the border of a result unit.
This value is passed to the ~ResultUnitFactory~.

*/

    void AddGlobalTimeValue(double t);

/*
1.1.1 AddToMRegion

Adds this unit to target.

*/

    void AddToMRegion(MRegion* const target) const;

/*
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

    void CollectRelevantPFaces(ResultUnitFactory* receiver);

/*
1.1.1 IsEntirelyOutside

Returns ~true~, if pFace is entirely outside of this ~SourceUnit~.

Precondition: pFace is already known as entirely inside or entirely outside!

*/

    bool IsEntirelyOutside(const PFace* pFace);

/*
1.1.1 IsOutside

Returns ~true~, if ~p~ is outside of this ~SourceUnit~.

Note: We create a ~TestRegion~ ~tr~ by evaluating this ~SourceUnit~
at time ~p.t~ and check, if the point ~(p.x; p.y)~ is outside of ~tr~.
For the sake of efficency, we cache ~tr~ for later use.

*/

    bool IsOutside(const Point3D& p);

/*
1.1.1 IsOnBorder

Returns ~true~, if ~p~ is on the border of this ~SourceUnit~.

Note: We create a ~TestRegion~ ~tr~ by evaluating this ~SourceUnit~
at time ~p.t~ and check, if the point ~(p.x; p.y)~ is on the border of ~tr~.
For the sake of efficency, we cache ~tr~ for later use.

*/

    bool IsOnBorder(const Point3D& p);

/*
1.1 Methods for debugging

*/

    void Print() const;
    void PrintPFaces();
    void PrintVRMLDesc(ofstream& target, const string& color);

private:

/*
1.1 Private methods

*/

    void NormalizeTimeInterval();
    void ComputeBoundingRect();
    const Region GetTestRegion(const double t);

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

    const bool isEmpty;

/*
1.1.1 uRegion

A pointer to the corresponding ~URegionEmb~.

Note that the interval of uRegion might be changed!

*/

    URegionEmb* const uRegion;

/*
1.1.1 array

A pointer to the corresponding ~DBArray<MSegmentData>~.

*/

    const DbArray<MSegmentData>* const array;

/*
1.1.1 partner

A pointer to  the other ~SourceUnit~ instance, which
is the other argument of the set operation.

*/

    SourceUnit* partner;

/*
1.1.1 parent

A pointer the parent object.

*/

    SourceUnitPair* const parent;

/*
1.1.1 startTime, endTime

The start/end of the interval as double value.

*/

    double startTime;
    double endTime;

/*
1.1.1 hasNormalizedTimeInterval

A flag, indicating if the interval of this ~SourceUnit~ was normalized to 1.0.
This might be useful to improve the numerical stability of very thin units.

*/

    bool hasNormalizedTimeInterval;

/*
1.1.1 originalTimeInterval

The original interval of this ~SourceUnit~ before normalization.

*/

    Interval<Instant> originalTimeInterval;

/*
1.1.1 pFaces

A ~std::vector~ to store all ~PFaces~.

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
1.1.1 boundingRect

The projection bounding rectangle of this ~SourceUnit~ to the xy-plane.

*/

    Rectangle<2> boundingRect;

/*
1.1.1 testRegion

The cached ~TestRegion~ used by the methods ~IsOutside~ and ~IsOnBorder~.

*/

    TestRegion testRegion;
};

/*
1 Enumeration Decision

Used in the class ~ResultUnitFactory~ to indicate,
if a brick of a ~PFace~ belongs to the result unit.

*/

enum Decision {

    UNDEFINED,
    ACCEPT,
    SKIP
};

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

    ResultUnitFactory(MRegion* const _resMRegion,
                      const SourceUnitPair* const _parent) :

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
1.1.1 AddTimeValue

Adds a relevant timevalue.

*/

    inline void AddTimeValue(double t) {

        time.insert(t);
    }

/*
1.1 Methods for debugging

*/

    void Print() const;
    string GetVRMLDesc() const;
    void AddToOverallStatistic() const;

private:

/*
1.1 Private methods

*/

    void ComputeCurrentTimeLevel();
    void ProcessNormalPFace(PFace* pFace);
    void ProcessCriticalPFace(PFace* pFace);
    void ConstructResultUnitAsURegionEmb();
    Decision BelongsToResultUnit(const Point3D& p, const PFace* pFace);
    void AddCriticalMSegments();

/*
1.1 Attributes

1.1.1 time, timeIter

A ~std::set~ to store the relevant timevalues and a suitable iterator.

*/

    set<double, DoubleCompare> time;
    set<double, DoubleCompare>::const_iterator timeIter;

/*
1.1.1 t1, t12, t2

The start-, middle- and endtime of the current ~ResultUnit~.

*/

    double t1;
    double t12;
    double t2;

/*
1.1.1 pFaces

A ~std::vector~ to store the relevant ~PFaces~.

*/

    vector<PFace*> pFaces;

/*
1.1.1 criticalMSegs

A ~std::vector~ to store the critical MSegments.

*/

    vector<MSegmentCritical> criticalMSegs;

/*
1.1.1 resMRegion

A pointer to the result ~MRegion~.

*/

    MRegion* resMRegion;

/*
1.1.1 parent

A pointer to the parent object.

*/

    const SourceUnitPair* const parent;

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
1 Class SourceUnitPair

This class is essentially a container for both ~SourceUnits~ A and B,
creates and initialise the ~ResultUnitFactory~ and runs the main-loop
of the set operation by calling the method ~Operate~.

*/

class SourceUnitPair {

public:

/*
1.1 Constructor

The constructor takes essentially two pointers to ~URegionEmb~ instances,
one pointer to the global result ~MRegion~ and an enum to indicate the
kind of set operation.

*/

    SourceUnitPair(URegionEmb* const unitA,
                   const DbArray<MSegmentData>* const aArray,
                   const bool aIsEmpty,
                   URegionEmb* const unitB,
                   const DbArray<MSegmentData>* const bArray,
                   const bool bIsEmpty,
                   const SetOp operation,
                   MRegion* const resultMRegion);

/*
1.1 Getter and setter methods

1.1.1 GetOperation

Returns an enum to indicate the current set operation:
INTERSECTION, UNION or MINUS.

*/

    inline const SetOp GetOperation() const {

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

/*
1.1.1 GetTimeInterval

Returns the common current interval of both ~SourceUnits~.

*/

    inline const Interval<Instant>& GetTimeInterval() const {

        return unitA.GetTimeInterval();
    }

/*
1.1.1 GetOriginalTimeInterval

Returns the common original interval of both ~SourceUnits~
before normalization.

*/

    inline const Interval<Instant>& GetOriginalTimeInterval() const {

        return unitA.GetOriginalTimeInterval();
    }

/*
1.1.1 GetStart/EndTime

Returns the start/end of the interval as double value.

*/

    inline double GetStartTime() const {

        return unitA.GetStartTime();
    }

    inline double GetEndTime() const {

        return unitA.GetEndTime();
    }

/*
1.1.1 HasNormalizedTimeInterval

Returns ~true~, if the interval was normalized to 1.0.
This might be useful to improve the numerical stability of very thin units.

*/

    inline bool HasNormalizedTimeInterval() const {

        return unitA.HasNormalizedTimeInterval();
    }

/*
1.1 Operators

1.1.1 Operate

This method runs the main-loop of the set operation by calling
the following methods:

  1 CreatePFaces

  2 ComputeIntSegs

  3 CollectRelevantPFaces

  4 ConstructResultUnits

*/

    void Operate();

/*
1.1.1 AddGlobalTimeValue

Adds a relevant timevalue, which marks the border of a result unit.
This value is passed to the ~ResultUnitFactory~.

*/

    inline void AddGlobalTimeValue(double t) {

        resultUnitFactory.AddTimeValue(t);
    }

/*
1.1 Methods for debugging

*/

    void PrintPFaces();
    void ToVrmlFile(bool a, bool b, bool res);

private:

/*
1.1 Private methods

1.1.1 CreatePFaces

Calls ~SourceUnit::CreatePFaces~ on both instances.

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

    void CollectRelevantPFaces();

/*
1.1.1 ConstructResultUnits

Runs the construction of all result units by calling
~ResultUnitFactory::Start~.

*/

    void ConstructResultUnits();

/*
1.1.1 ComputeOverlapRect

Computes the intersection of the projection bounding rectangles of
both ~SourceUnits~.

*/

    void ComputeOverlapRect();

/*
1.1 Attributes

*/

    SourceUnit unitA;
    SourceUnit unitB;
    const SetOp op;
    Rectangle<2> overlapRect;
    MRegion* const resultMRegion;
    ResultUnitFactory resultUnitFactory;

};

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

    IntersectionSegment(const Segment3D& s);

/*
1.1 Getter and setter methods

1.1.1 GetPFace

Returns a pointer to the ~PFace~, this instance belongs to.

*/

    inline const PFace* GetPFace() const {

        assert(pFace != 0);
        return pFace;
    }

/*
1.1.1 SetPFace

Initialise the field ~pFace~.

*/

    inline void SetPFace(const PFace* _pFace) {

        assert(pFace == 0);
        pFace = _pFace;
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

    inline const Point2D GetStartXY() const {

        return Point2D(startXYT);
    }

    inline double GetStartT() const {

        return startXYT.GetT();
    }

    inline const Point3D* GetEndXYT() const {

        return &endXYT;
    }

    inline const Point2D GetEndXY() const {

        return Point2D(endXYT);
    }

    inline double GetEndT() const {

        return endXYT.GetT();
    }

    inline const Point2D* GetStartWT() const {

        return &startWT;
    }

    inline const Point2D* GetEndWT() const {

        return &endWT;
    }

    inline double GetStartW() const {

        return startWT.GetW();
    }

    inline double GetEndW() const {

        return endWT.GetW();
    }

/*
1.1 Operators and Predicates

1.1.1 IsOrthogonalToTAxis

Returns ~true~, if this is parallel to the xy-plane.

*/

    inline bool IsOrthogonalToTAxis() const {

        return NumericUtil::NearlyEqual(GetStartT(), GetEndT());
    }

/*
1.1.1 IsLeftOf

Returns ~true~, if this is left of intSeg.

Preconditions:

  * $intSeg.startpoint.t <= this.startpoint.t <= intSeg.endpoint.t$

  * this and intSeg don't intersect in their interior.

*/

    bool IsLeftOf(const IntersectionSegment* intSeg) const;

/*
1.1.1 Evaluate

Returns the point (in xyt-coords) on this segment with t-coord t.

Precondition: $startpoint.t <= t <= endpoint.t$

*/

    Point3D Evaluate(const double t) const;

/*
1.1 Methods for debugging

*/

    inline unsigned int GetID() const {

        return id;
    }

    string GetVRMLDesc();
    void Print() const;

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

1.1.1 id

A unique id for each instance.
Used for debugging only.

*/

    unsigned int id;
	static unsigned int instanceCount;

/*
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

/*
1.1.1 pFace

A pointer to the ~PFace~, this instance belongs to.

*/

    const PFace* pFace;
};

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
1.1.1 GetStartAsListExpr/GetEndAsListExpr

Coverts this ~MSegment~ to ~ListExpr~.

*/

    inline const ListExpr GetStartAsListExpr() const {

        return NList(NList(initial.GetStart().GetX()),
                     NList(initial.GetStart().GetY()),
                     NList(final.GetStart().GetX()),
                     NList(final.GetStart().GetY())).listExpr();
    }

    inline const ListExpr GetEndAsListExpr() const {

        return NList(NList(initial.GetEnd().GetX()),
                     NList(initial.GetEnd().GetY()),
                     NList(final.GetEnd().GetX()),
                     NList(final.GetEnd().GetY())).listExpr();
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

/*
1 Class ResultUnit

This class essentially collects at first all ~MSegments~ of a result unit,
produced by the ~ResultUnitFactory~. Afterwards, the ~MSegments~ will be
suitably linked together, to form a proper ~URegionEmb~. This is done by
calling the method EndBulkLoad.

*/

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

/*
1.1.1 AddSegment

Adds a ~MSegment~ to this ~ResultUnit~.

*/

    inline void AddSegment(MSegment ms) {

        ms.SetSegmentNo(index++);

        ms.SetLeftDomPoint(true);
        msegments.push_back(ms);

        ms.SetLeftDomPoint(false);
        msegments.push_back(ms);
    }

/*
1.1.1 EndBulkLoad

This method must be called after adding the last ~MSegment~ and
performs the following tasks:

  * Sort the list ~ms~ of the ~MSegments~ by their median ~HalfSegments~,
    using ~MSegment::LessByMedianHS~ for comparison.

  * Construct a ~Region~ ~r~ from all median ~HalfSegments~ of ~ms~.

  * Copy the faceNo, cycleNo and edgeNo of each ~HalfSegment~ of ~r~ to
	the corresponding ~MSegment~ of ~ms~. Since ~r~ and ~ms~ has the same order
    this can be done by a linear scan.

  * Sort ~ms~ by faceno, cycleno and segmentno,
    using ~MSegment::LogicLess~ for comparison.

The parameter ~merge~ is not used yet.

*/

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

    inline bool IsEmpty() const {

        return msegments.size() == 0;
    }

/*
1.1 Methods for debugging

*/

    void Print(const bool segments = true) const;
    string GetVRMLDesc() const;
    bool Check() const;

private:

/*
1.1 Private methods

*/

    void AddMSegmentData(URegionEmb* uregion,
                         DbArray<MSegmentData>* segments,
                         MSegmentData& dms) const;

    static bool Less(const MSegment& ms1, const MSegment& ms2) {

        return ms1.LessByMedianHS(ms2);
    }

    static bool LogicLess(const MSegment& ms1, const MSegment& ms2) {

        return ms1.LogicLess(ms2);
    }

/*
1.1 Attributes

1.1.1 interval

The interval of this ~ResultUnit~

*/

    const Interval<Instant> interval;

/*
1.1.1 msegments

The list of the ~MSegments~.

*/

    vector<MSegment> msegments;

/*
1.1.1 index

A counter, used in ~AddSegment~.

*/

    unsigned int index;
};

/*
1 Enumeration SourceFlag

Indicates the source unit of a ~Point3DExt~.

*/

enum SourceFlag {

    PFACE_A,
    PFACE_B
};

/*
1 Class Point3DExt

This datastructure is used in the class ~PointExtSet~.
It simply extends ~Point3D~ with the attribute ~sourceFlag~.

*/

class Point3DExt : public Point3D {

public:

/*
1.1 Constructors

*/

    inline Point3DExt() :

        Point3D() {
    }

    inline Point3DExt(double a, double b, double c, SourceFlag _sourceFlag) :

        Point3D(a, b, c),
        sourceFlag(_sourceFlag) {
    }

/*
1.1 Operators and Predicates

1.1.1 operator $<$

*/

    bool operator <(const Point3DExt& p) const;

/*
1.1 Attributes

1.1.1 sourceFlag

An enum, indicating the ~PFace~, this ~Point3D~ belongs to.
Possible values are:

  * $PFACE\_A$

  * $PFACE\_B$

*/

    SourceFlag sourceFlag;
};

/*
1 Class PointExtSet

This set is used in the class ~PFace~ to compute the intersection segment of
two ~PFaces~.

*/

class PointExtSet {

public:

/*
1.1 Constructors

*/

    inline PointExtSet() {

    }

/*
1.1 Operators and Predicates

1.1.1 Insert

Inserts p, if p isn't already inserted.

*/

    inline void Insert(const Point3DExt& p) {

        s.insert(p);
    }

/*
1.1.1 Size

Returns the number of points in the set.

*/

    inline unsigned int Size() const {

        return s.size();
    }

/*
1.1.1 GetIntersectionSegment

Returns ~true~, if there is an intersection segment and writes it to result.

*/

    bool GetIntersectionSegment(Segment3D& result) const;

/*
1.1 Methods for debugging

*/

void Print() const;

private:

/*
1.1 Attributes

1.1.1 s

A ~std::set~, using the overloaded operator $<$ for comparison.

*/

    set<Point3DExt> s;
};

/*
1 Struct IntSegCompare

This struct implements the ~IntersectionSegment~ order,
used in ~IntSegContainer~.

*/
struct IntSegCompare {

    bool operator()(const IntersectionSegment* const& s1,
            const IntersectionSegment* const& s2) const;
};

/*
1 Class IntSegContainer

This class is used by the class ~PFace~ and provides essentially

  * an ordered set of ~IntersectionSegments~

  * an ordered multiset of critical timelevels

  * the ability to perform a plane-sweep in t-direction from bottom to top.

*/

class IntSegContainer {

public:

/*
1.1 Constructor

*/

    inline IntSegContainer() :

        firstTimeLevel(true),
        lastDecision(UNDEFINED) {

    }

/*
1.1 Destructor

*/

    ~IntSegContainer();

/*
1.1 Operators

1.1.1 AddIntSeg

Adds seg to the set of ~IntersectionSegments~.

*/

    inline void AddIntSeg(IntersectionSegment* seg) {

        intSegs.insert(seg);
    }

/*
1.1.1 AddCriticalTime

Adds t to the multiset of critical timelevels.

*/

    inline void AddCriticalTime(const double& t) {

        criticalTimes.insert(t);
    }

/*
1.1.1 GetActiveIntSegs

Returns a pointer to a list of the currently active ~IntersectionSegments~,
according to the previous call of ~UpdateTimeLevel~. An ~IntersectionSegment~ s
is active at time t, if the condition $s.startpoint.t <= t < s.endpoint.t$
holds.

*/

    inline const list<IntersectionSegment*>* GetActiveIntSegs() const {

        return &active;
    }

/*
1.1.1 UpdateTimeLevel

Updates the list of the currently active ~IntersectionSegments~: Insert
segments with $startpoint.t = t$ and remove segments with $endpoint.t = t$.

*/

    void UpdateTimeLevel(double t);

/*
1.1.1 SetLastDecision

Remembers the decision for the first brick of the current timelevel.

*/

    inline void SetLastDecision(const Decision d) {

        lastDecision = d;
    }

/*
1.1.1 GetLastDecision

Returns the correct decision for the first brick of a new timelevel.

Precondition: ~UpdateTimeLevel~ was already called.

*/

    inline const Decision GetLastDecision() const {

        if (lastDecision == UNDEFINED) {

            return UNDEFINED;
        }

        if (criticalTimes.count(t) % 2 == 0) {

            return lastDecision;
        }

        if (lastDecision == ACCEPT)
            return SKIP;
        else
            return ACCEPT;
    }

/*
1.1 Methods for debugging

*/

    void Print() const;
    void PrintActive() const;

private:

/*
1.1 Private methods

*/

    inline bool IsOutOfRange(IntersectionSegment* seg) const {

        return NumericUtil::NearlyEqual(seg->GetEndT(), t);
    }

    inline bool HasMoreSegsToInsert() const {

        return intSegIter != intSegs.end() &&
               NumericUtil::NearlyEqual((*intSegIter)->GetStartT(), t);
    }

/*
1.1 Attributes

1.1.1 intSegs, intSegIter

A ~std::set~ to store the ~IntersectionSegments~ using the order
provided by ~IntSegCompare~ and a suitable iterator.

*/

    set<IntersectionSegment*, IntSegCompare> intSegs;
    set<IntersectionSegment*, IntSegCompare>::iterator intSegIter;

/*
1.1.1 active, activeIter

A ~std::list~ to store the active ~IntersectionSegments~ during the plane-sweep
and a suitable iterator.

*/

    list<IntersectionSegment*> active;
    list<IntersectionSegment*>::iterator activeIter;

/*
1.1.1 criticalTimes

A ~std::multiset~ to store the the critical timelevels.

*/

    multiset<double, DoubleCompare> criticalTimes;

/*
1.1.1 t

The current timelevel.

*/

    double t;

/*
1.1.1 firstTimeLevel

A flag, indicating that the current timelevel is the lowest one.

*/

    bool firstTimeLevel;

/*
1.1.1 lastDecision

An enum, that stores the decision for the first brick of the current timelevel:

  * UNDEFINED

  * ACCEPT

  * SKIP

*/

    Decision lastDecision;
};

/*
1 Enumeration Boundary

Used in the class ~PFace~ to indicate the border.

*/

enum Boundary {

    LEFT_BOUNDARY,
    RIGHT_BOUNDARY
};

/*
1 Enumeration RelationToBoundary

Used in the class ~PFace~ to indicate the relation of an
~IntersectionSegment~ to the ~PFace~ boundary.

*/

enum RelationToBoundary {

    NO_TOUCH,
    TOUCH_IN_STARTPOINT,
    TOUCH_IN_ENDPOINT,
    COLINEAR
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

*/

    PFace(SourceUnit* _unit, const MSegmentData* _mSeg);

/*
1.1 Getter and setter methods

1.1.1 GetMSeg

Returns a pointer to the corresponding ~MSegmentData~ instance.

*/

    inline const MSegmentData* GetMSeg() const {

        return mSeg;
    }

/*
1.1.1 GetBoundingRect

Returns the projection bounding rectangle in xy-coords.

*/

    inline Rectangle<2> GetBoundingRect() const {

        return boundingRect;
    }

/*
1.1.1 GetStartTime/GetEndTime

Returns the min./max. value of t.

*/

    inline double GetStartTime() const {

        return unit->GetStartTime();
    }

    inline double GetEndTime() const {

        return unit->GetEndTime();
    }

/*
1.1.1 GetA XYT

Returns the lower left point in xyt-coords of the ~PFace~,
if viewed from outside.

*/

    inline Point3D GetA_XYT() const {

        if (InitialStartPointIsA())
            return Point3D(mSeg->GetInitialStartX(), mSeg->GetInitialStartY(),
                    unit->GetStartTime());
        else
            return Point3D(mSeg->GetInitialEndX(), mSeg->GetInitialEndY(),
                    unit->GetStartTime());
    }

/*
1.1.1 GetB XYT

Returns the lower right point in xyt-coords of the ~PFace~,
if viewed from outside.

*/

    inline Point3D GetB_XYT() const {

        if (InitialStartPointIsA())
            return Point3D(mSeg->GetInitialEndX(), mSeg->GetInitialEndY(),
                    unit->GetStartTime());
        else
            return Point3D(mSeg->GetInitialStartX(), mSeg->GetInitialStartY(),
                    unit->GetStartTime());
    }

/*
1.1.1 GetC XYT

Returns the upper left point in xyt-coords of the ~PFace~,
if viewed from outside.

*/

    inline Point3D GetC_XYT() const {

        if (InitialStartPointIsA())
            return Point3D(mSeg->GetFinalStartX(), mSeg->GetFinalStartY(),
                    unit->GetEndTime());
        else
            return Point3D(mSeg->GetFinalEndX(), mSeg->GetFinalEndY(),
                    unit->GetEndTime());
    }

/*
1.1.1 GetD XYT

Returns the upper right point in xyt-coords of the ~PFace~,
if viewed from outside.

*/

    inline Point3D GetD_XYT() const {

        if (InitialStartPointIsA())
            return Point3D(mSeg->GetFinalEndX(), mSeg->GetFinalEndY(),
                    unit->GetEndTime());
        else
            return Point3D(mSeg->GetFinalStartX(), mSeg->GetFinalStartY(),
                    unit->GetEndTime());
    }

/*
1.1.1 GetA WT

Returns the lower left point in wt-coords of the ~PFace~,
if viewed from outside.

*/

    inline Point2D GetA_WT() const {

        return Point2D(aW, unit->GetStartTime());
    }

/*
1.1.1 GetB WT

Returns the lower right point in wt-coords of the ~PFace~,
if viewed from outside.

*/

    inline Point2D GetB_WT() const {

        return Point2D(bW, unit->GetStartTime());
    }

/*
1.1.1 GetC WT

Returns the upper left point in wt-coords of the ~PFace~,
if viewed from outside.

*/

    inline Point2D GetC_WT() const {

        return Point2D(cW, unit->GetEndTime());
    }

/*
1.1.1 GetD WT

Returns the upper right point in wt-coords of the ~PFace~,
if viewed from outside.

*/

    inline Point2D GetD_WT() const {

        return Point2D(dW, unit->GetEndTime());
    }

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
1.1.1 GetBoundary

Returns a ~Segment2D~ which represents the left or right boundary
of this ~PFace~, as specified by the parameter b.
Note that the result depends on the kind of set operation.

*/

    inline Segment2D GetBoundary(const Boundary b) const {

        if (GetOperation() == MINUS && IsPartOfUnitB()) {

            if (b == LEFT_BOUNDARY)
                return Segment2D(GetA_WT(), GetC_WT());
            else
                return Segment2D(GetB_WT(), GetD_WT());

        } else {

            if (b == RIGHT_BOUNDARY)
                return Segment2D(GetA_WT(), GetC_WT());
            else
                return Segment2D(GetB_WT(), GetD_WT());
        }
    }

/*
1.1 Operators and Predicates

1.1.1 AEqualsB

Returns ~true~, if this ~PFace~ is a triangle and
the edge AB is degenerated to a point.

*/

    inline bool AEqualsB() const {

        return mSeg->GetPointInitial();
    }

/*
1.1.1 CEqualsD

Returns ~true~, if this ~PFace~ is a triangle and
the edge CD is degenerated to a point.

*/

    inline bool CEqualsD() const {

        return mSeg->GetPointFinal();
    }

/*
1.1.1 Intersection

Computes the intersection of this ~PFace~ with pf and adds
one ~IntersectionSegment~ instance to both ~PFaces~.

*/

    void Intersection(PFace& pf);

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
1.1.1 HasEqualNormalVector

Returns ~true~, if the normalvectors of this and pf are equal.

*/

    bool HasEqualNormalVector(const PFace& pf) const;

/*
1.1.1 TransformToWT

Transforms the argument from the xyt-system to the wt-system.

*/

    inline double TransformToW(const Point3D& xyt) const {

        return xyt.GetX() * GetWVector().GetX() +
               xyt.GetY() * GetWVector().GetY();
    }

    inline double TransformToW(const Vector3D& xyt) const {

        return xyt.GetX() * GetWVector().GetX() +
               xyt.GetY() * GetWVector().GetY();
    }

    inline Vector2D TransformToWT(const Vector3D& xyt) const {

        return Vector2D(TransformToW(xyt), xyt.GetT() );
    }

    inline Point2D TransformToWT(const Point3D& xyt) const {

        return Point2D(TransformToW(xyt), xyt.GetT());
    }

    inline Segment2D TransformToWT(const Segment3D& xyt) const {

        return Segment2D(TransformToWT(xyt.GetStart()),
                         TransformToWT(xyt.GetEnd()));
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

/*
1.1.1 GetUnit

Returns a pointer to the ~SourceUnit~ this ~PFace~ belongs to.

*/

    inline SourceUnit* GetUnit() const {

        return unit;
    }

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
1.1.1 GetState

Returns an enum to indicate the state of this ~PFace~.

*/

    inline State GetState() const {

        return state;
    }

/*
1.1.1 HasInnerIntSegs

Returns ~true~, if this ~PFace~ contains ~IntersectionSegments~ which are not
colinear to the boundary.

*/

    inline bool HasInnerIntSegs() const {

        return hasInnerIntSegs;
    }

/*
1.1.1 IsColinearToBoundary

Returns ~true~, if s is colinear to the left or right boundary of this ~PFace~.

*/

    bool IsColinearToBoundary(const Segment3D& s) const;

/*
1.1.1 GetMidPoint

Returns the midpoint of this ~PFace~.
Note that this point is never part of the boundary.

*/

    const Point3D GetMidPoint() const;

/*
1.1.1 GetRelationToBoundary

Returns an enum to indicate the spatial relationship of s and b.

*/

    const RelationToBoundary
    GetRelationToBoundary(const Segment3D& s, const Boundary b) const;

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
1.1.1 GetActiveIntSegs

Returns a pointer to a list of the currently active ~IntersectionSegments~,
according to the previous call of ~UpdateTimeLevel~. An ~IntersectionSegment~ s
is active at time t, if the condition $s.startpoint.t <= t < s.endpoint.t$
holds.

*/

    inline const list<IntersectionSegment*>* GetActiveIntSegs() const {

        return intSegs.GetActiveIntSegs();
    }

/*
1.1.1 UpdateTimeLevel

Updates the list of the currently active ~IntersectionSegments~: Insert
segments with $startpoint.t = t$ and remove segments with $endpoint.t = t$.

*/

    inline void UpdateTimeLevel(double t) {

        intSegs.UpdateTimeLevel(t);
    }

/*
1.1.1 SetLastDecision

Remembers the decision for the first brick of the current timelevel.

*/

    inline void SetLastDecision(const Decision d) {

        intSegs.SetLastDecision(d);
    }

/*
1.1.1 GetLastDecision

Returns the correct decision for the first brick of a new timelevel,
or UNDEFINED, if this ~PFace~ is critical.

Precondition: ~UpdateTimeLevel~ is already called.

*/

    inline const Decision GetLastDecision() const {

        if (IsCriticalRelevant())
            return UNDEFINED;

        return intSegs.GetLastDecision();
    }

/*
1.1 Methods for debugging

*/

    inline unsigned int GetID() const {

        return id;
    }

    string GetVRMLDesc();
    string GetStateAsString() const;
    void Print() const;

private:

/*
1.1 Private methods

*/

    inline bool InitialStartPointIsA() const {

        return initialStartPointIsA;
    }

    void SetInitialStartPointIsA();
    void ComputeBoundingRect();
    void ComputeNormalVector();
    void ComputeWTCoords();
    bool IntersectionPlaneSegment(const Segment3D& seg, Point3D& result);
    void DecideRelevanceAndAddBoundary();
    void AddEntireBoundary();
    void AddIntSeg(const Segment3D& seg, PFace& other);
    void AddBoundaryIntSeg(const Segment3D& seg);

/*
1.1 Attributes

1.1.1 unit

A pointer to the ~SourceUnit~, this ~PFace~ belongs to.

*/

    SourceUnit* const unit;

/*
1.1.1 mSeg

A pointer to the ~MSegmentData~, this ~PFace~ represents.

*/

    const MSegmentData* const mSeg;

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
1.1.1 wVector

The w-unitvector of this ~PFace~,
which defines the w-axis of the wt-system.

The vector w is either the normalized cross product
of the normal vector and the t-unit-vector, or it's negative.
This depends on the kind of set-operation, we want to perform.

*/

    Vector3D wVector;

/*
1.1.1 intSegs

A container to store all ~IntersectionSegments~ of this ~PFace~.

*/

    IntSegContainer intSegs;

/*
1.1.1 aW, bW, cW, dW

The w-coords of the four vertices of this ~PFace~.

*/

    double aW;
    double bW;
    double cW;
    double dW;

/*
1.1.1 initialStartPointIsA

A flag, indicating if the vertex A equals the ~initialStartPoint~ of the
corresponding ~MSegmentData~ instance.

*/

    bool initialStartPointIsA;

/*
1.1.1 hasInnerIntSegs

A flag, indicating if this ~PFace~ contains ~IntersectionSegments~ which are not
colinear to the boundary.

*/

    bool hasInnerIntSegs;

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
1.1.1 id

A unique id for each instance.
Used for debugging only.

*/

    unsigned int id;
    static unsigned int instanceCount;
};

} // end of namespace mregionops

} // end of namespace temporalalgebra

#endif /*SETOPS_H_*/

