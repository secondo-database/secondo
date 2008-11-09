/*

//paragraph [1] Title: [{\Large \bf \begin {center}] [\end {center}}]
//[TOC] [\tableofcontents]
//[ue] [\"u]
//[ae] [\"a]
//[oe] [\"o]

[1] 

April 2008, initial version created by M. H[oe]ger for bachelor thesis.

[TOC]

1 Introduction

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
    
    //UNDEFINED,
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

class Point3DExt : public Point3D {

public:

    inline Point3DExt() :
        
        Point3D() {
    }

    inline Point3DExt(double a, double b, double c, SourceFlag _sourceFlag) :
        
        Point3D(a, b, c), 
        sourceFlag(_sourceFlag) {
    }

    bool Lower(const Point3DExt& p) const;

    SourceFlag sourceFlag;
};

struct PointExtCompare {

    bool operator()(const Point3DExt& p1, const Point3DExt& p2) const {

        return p1.Lower(p2);
    }
};

class PointExtSet {

public:

    inline PointExtSet() {
        
    }

    inline void Insert(const Point3DExt& p) {

        //pair<set<PointExt>::iterator, bool> ret = s.insert(p);
        //return ret.second;

        s.insert(p);
    }

    inline unsigned int Size() const {

        return s.size();
    }

    bool GetIntersectionSegment(Segment3D& result) const;

private:

    set<Point3DExt, PointExtCompare> s;
};

class IntersectionSegment {

public:
    
    //IntersectionSegment(const Point3D& a, const Point3D& b);
    IntersectionSegment(const Segment3D& s);
    
    inline unsigned int GetID() const {

        return id;
    }
    
    inline const PFace* GetPFace() const {
        
        assert(pFace != 0);
        return pFace;
    }
    
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

    inline bool IsOrthogonalToTAxis() const {

        return NumericUtil::NearlyEqual(GetStartT(), GetEndT());
    }
    
    bool IsLeftOf(const IntersectionSegment* s) const;
    
    inline bool ResultFaceIsLeft() const {

        return sideOfResultFace == LEFT;
    }

    inline bool ResultFaceIsRight() const {

        return sideOfResultFace == RIGHT;
    }

    void SetWCoords();
    
    void SetSideOfResultFace(const PFace& self, const PFace& other);
    
    Point3D Evaluate(const double t) const;
    
    inline void SetPFace(const PFace* _pFace) {
        
        assert(pFace == 0);
        pFace = _pFace;
    }
    
    inline void SetStartWT(const Point2D& _startWT) {

        startWT = _startWT;
    }

    inline void SetEndWT(const Point2D& _endWT) {

        endWT = _endWT;
    }

    string GetVRMLDesc();
    void Print() const;

private:
    
    inline void SetResultFaceIsLeft() {
        
        assert(sideOfResultFace == UNK);

        sideOfResultFace = LEFT;
    }

    inline void SetResultFaceIsRight() {
        
        assert(sideOfResultFace == UNK);

        sideOfResultFace = RIGHT;
    }
    
    inline void FlipSideOfResultFace() {
        
        assert(sideOfResultFace != UNK);
        
        if (sideOfResultFace == LEFT)
            sideOfResultFace = RIGHT;
        else
            sideOfResultFace = LEFT;
    }
    
    unsigned int id;

    Point3D startXYT;
    Point3D endXYT;
    Point2D startWT;
    Point2D endWT;
    
    const PFace* pFace;
    
    SideOfResultFace sideOfResultFace;
    
    static unsigned int instanceCount;
};

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


struct DoubleCompare {

    bool operator()(const double& d1, const double& d2) const {
        
        return NumericUtil::Lower(d1, d2);
    }
};

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
    //void ComputeMedianRegion();
    const Region GetTestRegion(const double t);
    
    const bool isUnitA;
    const bool isEmpty;

    URegionEmb* const uRegion;
    const DBArray<MSegmentData>* const array;
    
    SourceUnit* partner;
    SourceUnitPair* const parent;
    
    double startTime;
    double endTime;
    //Instant medianTime;
    
    bool hasNormalizedTimeInterval;
    Interval<Instant> originalTimeInterval;
    //double scaleFactor;
    
    vector<PFace*> pFacesReduced;
    vector<PFace*> pFaces;
    
    Rectangle<2> boundingRect;
    
    //TestRegion medianRegion;
    TestRegion testRegion;
};

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

    ~SourceUnitPair() {

    }

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

class SetOperator {
    
public:
    
    SetOperator(MRegion* const _a, 
                MRegion* const _b, 
                MRegion* const _res) :
                
                a(_a), b(_b), res(_res) {

        //NumericUtil::SetEpsToDefault();
        //a_ref = new MRegion(0);
        //b_ref = new MRegion(0);
    }
    
    ~SetOperator() {
        
        //delete a_ref;
        //delete b_ref;
    }
    
    void Intersection();
    void Union();
    void Minus();
    
private:
    
    //void ComputeRefinementPartition();
    void Operate(const SetOp op);

    MRegion* const a;
    MRegion* const b;
    MRegion* const res;
    
    //MRegion* a_ref;
    //MRegion* b_ref;
};

struct IntSegCompare {

    bool operator()(const IntersectionSegment* const& s1,
            const IntersectionSegment* const& s2) const;
};

class IntSegContainer {
    
public:
    
    inline IntSegContainer() :
        
        firstTimeLevel(true),
        lastDecision(UNDEFINED) {
        
    }
    
    ~IntSegContainer();
    
    inline void AddIntSeg(IntersectionSegment* seg) {
        
        intSegs.insert(seg);
    }
    
    inline void AddCriticalTime(const double& t) {

        criticalTimes.insert(t);
    }
    
    inline const list<IntersectionSegment*>* GetActiveIntSegs() const {

        return &active;
    }
    
    void UpdateTimeLevel(double t);
    
    inline void SetLastDecision(const Decision d) {
        
        lastDecision = d;
    }
    
    inline const Decision GetLastDecision() const {
        
        // Precondition: UpdateTimeLevel is already called!
        
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
    
    void Print() const;
    void PrintActive() const;
    
private:
    
    inline bool IsOutOfRange(IntersectionSegment* seg) const {

        return NumericUtil::NearlyEqual(seg->GetEndT(), t);
        //return fabs(seg->GetEndT() - t) < 0.01;
    }

    inline bool HasMoreSegsToInsert() const {

        return intSegIter != intSegs.end() && 
               //fabs((*intSegIter)->GetStartT() - t) < 0.01;
               NumericUtil::NearlyEqual((*intSegIter)->GetStartT(), t);
    }
    
    inline const IntersectionSegment* GetSecondActiveIntSeg() const {
        
        assert(active.size() >= 2);
        
        list<IntersectionSegment*>::const_iterator iter = active.begin();
        iter++;
        
        return *iter;
    }
    
    set<IntersectionSegment*, IntSegCompare> intSegs;
    set<IntersectionSegment*, IntSegCompare>::iterator intSegIter;

    list<IntersectionSegment*> active;
    list<IntersectionSegment*>::iterator activeIter;
    
    multiset<double, DoubleCompare> criticalTimes;
    
    double t;
    bool firstTimeLevel;
    Decision lastDecision;
};

class PFace {

public:

    PFace(SourceUnit* _unit, const MSegmentData* _mSeg);
          
    ~PFace() {

    }

    inline const MSegmentData* GetMSeg() const {

        return mSeg;
    }

    inline Rectangle<2> GetBoundingRect() const {

        return boundingRect;
    }
    
    inline double GetStartTime() const {
        
        return unit->GetStartTime();
    }
    
    inline double GetEndTime() const {

        return unit->GetEndTime();
    }

    inline Point3D GetA_XYT() const {

        if (InitialStartPointIsA())
            return Point3D(mSeg->GetInitialStartX(), mSeg->GetInitialStartY(),
                    unit->GetStartTime());
        else
            return Point3D(mSeg->GetInitialEndX(), mSeg->GetInitialEndY(),
                    unit->GetStartTime());
    }

    inline Point3D GetB_XYT() const {

        if (InitialStartPointIsA())
            return Point3D(mSeg->GetInitialEndX(), mSeg->GetInitialEndY(),
                    unit->GetStartTime());
        else
            return Point3D(mSeg->GetInitialStartX(), mSeg->GetInitialStartY(),
                    unit->GetStartTime());
    }

    inline Point3D GetC_XYT() const {

        if (InitialStartPointIsA())
            return Point3D(mSeg->GetFinalStartX(), mSeg->GetFinalStartY(),
                    unit->GetEndTime());
        else
            return Point3D(mSeg->GetFinalEndX(), mSeg->GetFinalEndY(),
                    unit->GetEndTime());
    }

    inline Point3D GetD_XYT() const {

        if (InitialStartPointIsA())
            return Point3D(mSeg->GetFinalEndX(), mSeg->GetFinalEndY(),
                    unit->GetEndTime());
        else
            return Point3D(mSeg->GetFinalStartX(), mSeg->GetFinalStartY(),
                    unit->GetEndTime());
    }

    inline bool AEqualsB() const {

        return mSeg->GetPointInitial();
    }

    inline bool CEqualsD() const {

        return mSeg->GetPointFinal();
    }

    inline Point2D GetA_WT() const {

        return Point2D(aW, unit->GetStartTime());
    }

    inline Point2D GetB_WT() const {

        return Point2D(bW, unit->GetStartTime());
    }

    inline Point2D GetC_WT() const {

        return Point2D(cW, unit->GetEndTime());
    }

    inline Point2D GetD_WT() const {

        return Point2D(dW, unit->GetEndTime());
    }

    inline Vector3D GetNormalVector() const {

        return normalVector;
    }
    
    inline const SetOp GetOperation() const {
        
        return unit->GetOperation();
    }

    inline bool InitialStartPointIsA() const {

        return initialStartPointIsA;
    }

    inline Vector3D GetWVector() const {

        return wVector;
    }
    
    inline Vector3D GetNormalVectorOfResultFace() const {
        
        if (GetOperation() == MINUS && IsPartOfUnitB())
            return - GetNormalVector();
        else
            return GetNormalVector();
    }
    
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
    
    inline bool NormalVectorsHasSameDirection() const {

        return !(GetOperation() == MINUS && IsPartOfUnitB());
    }

    void Intersection(PFace& pf);

    bool IntersectionPlaneSegment(const Segment3D& seg, Point3D& result);

    bool IsParallelTo(const PFace& pf) const;
    bool IsCoplanarTo(const PFace& pf) const;
    bool IsCoplanarTo(const Point3D& p) const;
    bool IsCoplanarTo(const Segment3D& s) const;
    bool IsCoplanarTo(const IntersectionSegment& s) const;
    //bool IsEqual(const PFace& pf) const;
    bool HasEqualNormalVector(const PFace& pf) const;

    

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
    
    inline bool ResultVolumeIsRightOf(const Segment3D& s) const {

        Point2D startWT = TransformToWT(s.GetStart());
        Point2D endWT = TransformToWT(s.GetEnd());

        if (startWT < endWT)
            return !NormalVectorsHasSameDirection();
        else
            return NormalVectorsHasSameDirection();
    }

    inline void MarkAsEntirelyOutside() {

        state = ENTIRELY_OUTSIDE;
    }

    inline bool IsEntirelyOutside() const {

        return state == ENTIRELY_OUTSIDE;
    }

    inline void MarkAsEntirelyInside() {

        state = ENTIRELY_INSIDE;
    }

    inline bool IsEntirelyInside() const {

        return state == ENTIRELY_INSIDE;
    }
    
    inline void MarkAsNormalRelevant() {

        if (!IsCriticalRelevant())
            state = RELEVANT_NOT_CRITICAL;
    }
    
    inline void MarkAsCriticalRelevant() {

        state = RELEVANT_CRITICAL;
    }
    
    inline void MarkAsIrrelevant() {

        state = NOT_RELEVANT;
    }
    
    inline bool IsNormalRelevant() const {

        return state == RELEVANT_NOT_CRITICAL;
    }

    inline bool IsCriticalRelevant() const {

        return state == RELEVANT_CRITICAL;
    }

    inline bool IsIrrelevant() const {

        return state == NOT_RELEVANT;
    }
    
    inline SourceUnit* GetUnit() const {
        
        return unit;
    }

    inline bool IsPartOfUnitA() const {

        return unit->IsUnitA();
    }

    inline bool IsPartOfUnitB() const {

        return !unit->IsUnitA();
    }
    
    inline State GetState() const {
        
        return state;
    }
    
    inline bool HasInnerIntSegs() const {
        
        return hasInnerIntSegs;
    }
    
    inline const list<IntersectionSegment*>* GetActiveIntSegs() const {

        return intSegs.GetActiveIntSegs();
    }
    
    inline void UpdateTimeLevel(double t) {
        
        intSegs.UpdateTimeLevel(t);
    }
    
    inline void SetLastDecision(const Decision d) {

        intSegs.SetLastDecision(d);
    }

    inline const Decision GetLastDecision() const {

        // Precondition: UpdateTimeLevel is already called!
        
        if (IsCriticalRelevant())
            return UNDEFINED;

        return intSegs.GetLastDecision();
    }
    
    inline void Finalize() {
        
        AddBoundary();
    }
    
    bool IsColinearToBoundary(const Segment3D& s) const;
    
    const Point3D GetMidPoint() const;
    
    const RelationToBoundary 
    GetRelationToBoundary(const Segment3D& s, const Boundary b) const;
    
    inline unsigned int GetID() const {
        
        return id;
    }

    string GetVRMLDesc();
    void Print() const;

private:
    
    void SetInitialStartPointIsA();
    void ComputeBoundingRect();
    void ComputeNormalVector();
    void ComputeWTCoords();
    void AddBoundary();
    void AddEntireBoundary();
    void AddIntSeg(const Segment3D& seg, PFace& other);
    //void AddTouchSeg(const Segment3D& seg);
    void AddBoundaryIntSeg(const Segment3D& seg);

    SourceUnit* const unit;
    const MSegmentData* const mSeg;
    Rectangle<2> boundingRect;
    Vector3D normalVector;
    Vector3D wVector;

    IntSegContainer intSegs;

    double aW;
    double bW;
    double cW;
    double dW;

    //bool insideAbove;
    bool initialStartPointIsA;
    bool hasInnerIntSegs;
     
    State state;
    
    unsigned int id;
    static unsigned int instanceCount;
};

} // end of namespace mregionops

#endif /*SETOPS_H_*/


