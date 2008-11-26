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
provide the core functionality of the three set operators 
~intersection~, ~union~ and ~minus~ with the signature \\
movingregion [x] movingregion [->] movingregion \\
used in the MovingRegion Algebra.

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
#include "DateTime.h"
#include "StopWatch.h"
#include <vector>
#include <set>
#include <list>

using namespace std;

namespace mregionops {

#define OPTIMIZE_BY_BOUNDING_RECT
#define CACHE_TESTREGIONS
const bool NORMALIZE_TIME_INTERVAL_OF_SOURCE_UNITS = true;
//const bool SORT_RESULT_MSEGMENTS = false;
const bool MERGE_RESULT_MSEGMENTS = false;
const bool DECIDE_BY_ENTIRELY_IN_OUT = true;
const bool DECIDE_BY_PLUMBLINE_ONLY = false;

//#define VRML_OUTFILE "out.wrl"

static double seconds = 0.0;
static double seconds1 = 0.0;
static double seconds2 = 0.0;
static double seconds3 = 0.0;
static double seconds4 = 0.0;
static double seconds5 = 0.0;
static double seconds6 = 0.0;

enum SetOp {

    INTERSECTION,
    UNION,
    MINUS
};

enum SourceFlag {

    PFACE_A,
    PFACE_B
};

enum Boundary {
    
    LEFT_BOUNDARY,
    RIGHT_BOUNDARY
};

enum RelationToBoundary {
    
    NO_TOUCH,
    TOUCH_IN_STARTPOINT,
    TOUCH_IN_ENDPOINT,
    COLINEAR
};

enum State {
    
    UNKNOWN,
    ENTIRELY_INSIDE,
    ENTIRELY_OUTSIDE,
    RELEVANT_NOT_CRITICAL,
    RELEVANT_CRITICAL,
    NOT_RELEVANT
};

enum SideOfResultFace {
    
    UNK,
    LEFT,
    RIGHT
};

enum Decision {
    
    UNDEFINED,
    ACCEPT,
    SKIP
};

class Point3DExt;
class PointExtSet;
class IntersectionSegment;
class PFace;
class SourceUnit;
class SourceUnitPair;

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
1.1 Comparison method

*/

    bool Lower(const Point3DExt& p) const;

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
1 Struct PointExtCompare

*/

struct PointExtCompare {

    bool operator()(const Point3DExt& p1, const Point3DExt& p2) const {

        return p1.Lower(p2);
    }
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

private:

/*
1.1 Attributes

1.1.1 s

A ~std::set~ which uses the struct ~PointExtCompare~ for comparison.

*/  

    set<Point3DExt, PointExtCompare> s;
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



*/
class MSegment {
    
public:
    
    MSegment(const Segment3D& _initial,
             const Segment3D& _median,
             const Segment3D& _final,
             const PFace* const _pFace);
    
    inline const PFace* GetPFace() const {

        return pFace;
    }
    
    inline const Segment2D& GetInitial() const {

        return initial;
    }
    
    inline const Segment2D& GetFinal() const {

        return final;
    }
    
    inline const Segment2D& GetMedian() const {

        return median;
    }
    
    inline int GetFaceNo() const {
        
        return medianHS.attr.faceno;
    }
    
    inline int GetCycleNo() const {

        return medianHS.attr.cycleno;
    }

    inline int GetSegmentNo() const {

        return medianHS.attr.edgeno;
    }
    
    inline int GetInsideAbove() const {

        //return medianHS.attr.insideAbove;
        return insideAbove;
    }
    
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
    
    inline const HalfSegment& GetMedianHS() const {
        
        return medianHS;
    }
    
    inline bool IsLeftDomPoint() const {
        
        return medianHS.IsLeftDomPoint();
    }
    
    inline void SetFaceNo(int fn) {

        medianHS.attr.faceno = fn;
    }

    inline void SetCycleNo(int cn) {

        medianHS.attr.cycleno = cn;
    }
    
    inline void SetSegmentNo(int sn) {
        
        medianHS.attr.edgeno = sn;
    }
    
    inline void SetLeftDomPoint(bool ldp) {
        
        medianHS.SetLeftDomPoint(ldp);
    }
    
    inline void CopyIndicesFrom(const HalfSegment* hs) {
        
        medianHS.attr.faceno = hs->attr.faceno;
        medianHS.attr.cycleno = hs->attr.cycleno;
        medianHS.attr.edgeno = hs->attr.edgeno;
        //medianHS.attr.insideAbove = hs->attr.insideAbove;
    }
    
    inline void Flip() {
        
        initial.Flip();
        median.Flip();
        final.Flip();
        
        insideAbove = !insideAbove;
    }
    
    bool IsParallel(const MSegment& ms) const;
    
    inline bool LessByMedianHS(const MSegment& ms) const {
        
        return GetMedianHS() < ms.GetMedianHS();
    }
    
    inline bool LogicLess(const MSegment& ms) const {
        
        if (IsLeftDomPoint() != ms.IsLeftDomPoint())
            return IsLeftDomPoint() > ms.IsLeftDomPoint();
        
        return GetMedianHS().LogicCompare(ms.GetMedianHS()) == -1;
    }
    
    inline bool LessByMedianStartPoint(const MSegment& ms) const {

        return GetMedian().GetStart() < ms.GetMedian().GetStart();
    }
    
    void Print() const {
        
        cout << "start: " << NList(GetStartAsListExpr()) << endl;
        cout << "end: " << NList(GetEndAsListExpr()) << endl;
        cout << "median: " << this->GetMedianHS() << endl;
        cout << " F: " << GetFaceNo();
        cout << " C: " << GetCycleNo();
        cout << " S: " << GetSegmentNo();
        cout << " LDP: " << GetMedianHS().IsLeftDomPoint() << endl;
    }

private:
    
    Segment2D initial;
    Segment2D median;
    Segment2D final;
    
    HalfSegment medianHS;
    
    bool insideAbove;
    
    const PFace* pFace;
};

/*
1 Class MSegmentCritical



*/
class MSegmentCritical : public MSegment {
    
public:
    
    MSegmentCritical(const Segment3D& _initial,
                     const Segment3D& _median,
                     const Segment3D& _final,
                     const Point3D& _midPoint,
                     const PFace* const _pFace) :
                         
                         MSegment(_initial, _median, _final, _pFace),
                         midPoint(_midPoint) {
         
    }
    
    inline const Point2D& GetMidpoint() const {
        
        return midPoint;
    }
    
    bool IsPartOfUnitA() const;
    bool HasEqualNormalVector(const MSegmentCritical& msc) const;
    
    bool operator <(const MSegmentCritical& msc) const {

        return midPoint < msc.midPoint;
    }
    
private:
    
    Point2D midPoint;
};

/*
1 Class ResultUnit



*/
class ResultUnit {
    
public:
    
    ResultUnit(const Interval<Instant> _interval) :
        interval(_interval),
        index(0) {
        
    }
    
    inline const Interval<Instant>& GetInterval() const {
        
        return interval;
    }
    
    inline void StartBulkLoad() {
        
        index = 0;
    }
    
    inline void AddSegment(MSegment ms) {
        
        ms.SetSegmentNo(index++);
        
        ms.SetLeftDomPoint(true);
        msegments.push_back(ms);
        
        ms.SetLeftDomPoint(false);
        msegments.push_back(ms);
    }
    
    void EndBulkLoad(bool merge);
    
    const ListExpr ConvertToListExpr() const;
    
    URegionEmb* ConvertToURegionEmb(DBArray<MSegmentData>* segments) const;
    
    inline bool IsEmpty() const {
        
        return msegments.size() == 0;
    }
    
    void Print(const bool segments = true) const;
    
    void WriteToVRMLFile() const;
    
private:
    
    void AddMSegmentData(URegionEmb* uregion,
                         DBArray<MSegmentData>* segments, 
                         MSegmentData& dms) const;
    
    static bool Less(const MSegment& ms1, const MSegment& ms2) {

        return ms1.LessByMedianHS(ms2);
    }
    
    static bool LogicLess(const MSegment& ms1, const MSegment& ms2) {

        return ms1.LogicLess(ms2);
    }
    
    bool Check() const;
    
    const Interval<Instant> interval;
    vector<MSegment> msegments;
    unsigned int index;
    
    //vector<ResultFace> faces;
};

/*
1 Struct DoubleCompare

*/

struct DoubleCompare {

    bool operator()(const double& d1, const double& d2) const {
        
        return NumericUtil::Lower(d1, d2);
    }
};

/*
1 Class ResultUnitFactory



*/

class ResultUnitFactory {

public:

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
        decisionsBySideOfResultFace(0),
        decisionsByDegeneration(0) {
        
    }

    void Start();
    
    inline void AddPFace(PFace* pf) {
        
        pFaces.push_back(pf);
    }
    
    inline void AddTimeValue(double t) {

        time.insert(t);
    }
    
    void Print() const;
   
private:

    void ComputeCurrentTimeLevel();
    void ProcessNormalPFace(PFace* pFace);
    void ProcessCriticalPFace(PFace* pFace);
    void ConstructResultUnitAsURegionEmb();
    Decision BelongsToResultUnit(const Point3D& p, const PFace* pFace);
    void AddCriticalMSegments();

    set<double, DoubleCompare> time;
    set<double, DoubleCompare>::const_iterator timeIter;

    double t1;
    double t12;
    double t2;
    
    vector<PFace*> pFaces;
    vector<MSegmentCritical> criticalMSegs;
    
    MRegion* resMRegion;
    const SourceUnitPair* const parent;
    ResultUnit* resultUnit;
    
    unsigned int noUnits;
    unsigned int noEmptyUnits;
    unsigned int evalutedIntSegs;
    unsigned int noMSegsValid;
    unsigned int noMSegsSkipped;
    unsigned int noMSegCritical;
    unsigned int decisionsByPlumbline;
    unsigned int decisionsByEntirelyInOut;
    unsigned int decisionsBySideOfResultFace;
    unsigned int decisionsByDegeneration;
};

/*
1 Struct TestRegion



*/

struct TestRegion {
    
    TestRegion() :
        
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



*/
class SourceUnit {

    friend class SourceUnitPair;

public:

    SourceUnit(const bool _isUnitA, 
               URegionEmb* const _uRegion,
               const DBArray<MSegmentData>* const _array,
               const bool _isEmpty,
               SourceUnitPair* const _parent);
    
    ~SourceUnit();

    inline void SetPartner(SourceUnit* _partner) {

        partner = _partner;
    }
    
    inline SourceUnit* GetPartner() const {

        return partner;
    }

    inline SourceUnitPair* GetParent() const {

        return parent;
    }

    inline const URegionEmb* GetURegionEmb() const {

        return uRegion;
    }

    inline bool IsUnitA() const {

        return isUnitA;
    }
    
    inline bool IsUnitB() const {

        return !isUnitA;
    }
    
    const SetOp GetOperation() const;
    
    inline bool IsEmpty() const {
        
        return isEmpty;
    }
    
    inline bool HasNormalizedTimeInterval() const {
        
        return hasNormalizedTimeInterval;
    }
    
    inline const Interval<Instant>& GetTimeInterval() const {

        return uRegion->timeInterval;
    }
    
    inline const Interval<Instant>& GetOriginalTimeInterval() const {

        return originalTimeInterval;
    }

    inline double GetStartTime() const {

        return startTime;
    }

    inline double GetEndTime() const {

        return endTime;
    }

    inline Rectangle<2> GetBoundingRect() const {

        return boundingRect;
    }

    inline unsigned int GetNoOfPFaces() const {

        return pFaces.size();
    }
    
    void AddGlobalTimeValue(double t);
    
    void AddToMRegion(MRegion* const target) const;

    void CreatePFaces();
    void CollectRelevantPFaces(ResultUnitFactory* receiver);
    
    bool IsEntirelyOutside(const PFace* pFace);
    bool IsOutside(const Point3D& p);
    bool IsOnBorder(const Point3D& p);

    void Print() const;
    void PrintPFaces();
    void PrintVRMLDesc(ofstream& target, const string& color);

private:

    void NormalizeTimeInterval();
    void ComputeBoundingRect();
    const Region GetTestRegion(const double t);
    
    const bool isUnitA;
    const bool isEmpty;

    URegionEmb* const uRegion;
    const DBArray<MSegmentData>* const array;
    
    SourceUnit* partner;
    SourceUnitPair* const parent;
    
    double startTime;
    double endTime;
    
    bool hasNormalizedTimeInterval;
    Interval<Instant> originalTimeInterval;
    
    vector<PFace*> pFacesReduced;
    vector<PFace*> pFaces;
    
    Rectangle<2> boundingRect;
    
    TestRegion testRegion;
};

/*
1 Class SourceUnitPair



*/
class SourceUnitPair {

public:

    SourceUnitPair(URegionEmb* const unitA, 
                   const DBArray<MSegmentData>* const aArray,
                   const bool aIsEmpty,
                   URegionEmb* const unitB, 
                   const DBArray<MSegmentData>* const bArray,
                   const bool bIsEmpty,
                   const SetOp operation,
                   MRegion* const resultMRegion);

    void Operate();
    
    inline const SetOp GetOperation() const {
        
        return op;
    }

    inline Rectangle<2> GetOverlapRect() const {

        return overlapRect;
    }

    inline bool HasOverlappingBoundingRect() const {

        return overlapRect.IsDefined();
    }

    inline const Interval<Instant>& GetTimeInterval() const {

        return unitA.GetTimeInterval();
    }
    
    inline const Interval<Instant>& GetOriginalTimeInterval() const {

        return unitA.GetOriginalTimeInterval();
    }

    inline double GetStartTime() const {

        return unitA.GetStartTime();
    }

    inline double GetEndTime() const {

        return unitA.GetEndTime();
    }
    
    inline bool HasNormalizedTimeInterval() const {

        return unitA.HasNormalizedTimeInterval();
    }

    inline void AddGlobalTimeValue(double t) {

        resultUnitFactory.AddTimeValue(t);
    }
    
private:

    
    void ComputeOverlapRect();
    void CreatePFaces();
    void ComputeIntSegs();
    void CollectRelevantPFaces();
    void ConstructResultUnits();
    
    void PrintPFaces();
    void PrintIntSegsOfPFaces();
    void PrintUnitsAsVRML(bool a, bool b, bool res);
    
    SourceUnit unitA;
    SourceUnit unitB;
    const SetOp op;
    Rectangle<2> overlapRect;
    
    
    
    MRegion* const resultMRegion;
    ResultUnitFactory resultUnitFactory;
};

/*
1 Class SetOperator

This class provides the three set operations 
~intersection~, ~union~ and ~minus~ with the signature
movingregion [x] movingregion [->] movingregion.

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

Marks this ~PFace~ as entirely outside the other movingregion.

*/ 
    
    inline void MarkAsEntirelyOutside() {

        state = ENTIRELY_OUTSIDE;
    }

/*
1.1.1 IsEntirelyOutside

Returns ~true~, if this ~PFace~ is entirely outside the other movingregion.

*/ 

    inline bool IsEntirelyOutside() const {

        return state == ENTIRELY_OUTSIDE;
    }

/*
1.1.1 MarkAsEntirelyInside

Marks this ~PFace~ as entirely inside the other movingregion.

*/ 

    inline void MarkAsEntirelyInside() {

        state = ENTIRELY_INSIDE;
    }

/*
1.1.1 IsEntirelyInside

Returns ~true~, if this ~PFace~ is entirely inside the other movingregion.

*/ 

    inline bool IsEntirelyInside() const {

        return state == ENTIRELY_INSIDE;
    }

/*
1.1.1 MarkAsNormalRelevant

Marks this ~PFace~ as normal relevant for the result movingregion.
This means, that no critical case occured during the intersection process.

*/ 
    
    inline void MarkAsNormalRelevant() {

        if (!IsCriticalRelevant())
            state = RELEVANT_NOT_CRITICAL;
    }

/*
1.1.1 MarkAsCriticalRelevant

Marks this ~PFace~ as critical relevant for the result movingregion.
This means, that at least one critical case occured during the 
intersection process.

*/ 
    
    inline void MarkAsCriticalRelevant() {

        state = RELEVANT_CRITICAL;
    }

/*
1.1.1 MarkAsIrrelevant

Marks this ~PFace~ as irrelevant for the result movingregion.

*/ 
    
    inline void MarkAsIrrelevant() {

        state = NOT_RELEVANT;
    }

/*
1.1.1 IsNormalRelevant

Returns ~true~, if this ~PFace~ is normal relevant for the result movingregion.

*/ 

    inline bool IsNormalRelevant() const {

        return state == RELEVANT_NOT_CRITICAL;
    }

/*
1.1.1 IsCriticalRelevant

Returns ~true~, if this ~PFace~ is critical relevant 
for the result movingregion.

*/ 

    inline bool IsCriticalRelevant() const {

        return state == RELEVANT_CRITICAL;
    }

/*
1.1.1 IsIrrelevant

Returns ~true~, if this ~PFace~ is irrelevant for the result movingregion.

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
        
        AddBoundary();
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
    void AddBoundary();
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

#endif /*SETOPS_H_*/

