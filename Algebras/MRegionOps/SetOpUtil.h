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

#ifndef SETOPUTIL_H_
#define SETOPUTIL_H_

#include "MovingRegionAlgebra.h"
//#include "point.h"
//#include "vector.h"
#include "NumericUtil.h"
#include "PointVector.h"
#include "Segment.h"
#include "DateTime.h"
#include <vector>
#include <set>
#include <list>
#include <queue>
#include <deque>

namespace mregionops {

#define REDUCE_PFACES_BY_BOUNDING_RECT
#define VRML_OUTFILE "out.wrl"

enum SetOp {

    INTERSECTION,
    UNION,
    MINUS
};

enum SourceFlag {

    PFACE_A,
    PFACE_B
};

enum TouchingMode {

    NO_TOUCH = 0,
    TOUCH_IN_STARTPOINT = 1,
    TOUCH_IN_ENDPOINT = 2,
    UNDEFINED = 3
};

enum InsideOutside {

    UNKNOWN,
    INSIDE,
    OUTSIDE
};

class Point3DExt;
class PointExtSet;
class IntersectionSegment;
class PFace;
class PFaceCycle;
class PFaceIterator;
class PFacePair;
class SourceUnit;
class SourceUnitPair;

class Point3DExt : public Point3D {

public:

    inline Point3DExt() :
        Point3D() {
    }

    inline Point3DExt(double a, double b, double c, SourceFlag _sourceFlag) :
        Point3D(a, b, c), sourceFlag(_sourceFlag) {
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

    pair<IntersectionSegment*, IntersectionSegment*>
            GetIntersectionSegment() const;

private:

    set<Point3DExt, PointExtCompare> s;
};



class IntersectionSegment {

public:

    inline ~IntersectionSegment() {

    }

    inline const Point3D* GetStartXYT() const {

        return startXYT;
    }

    inline double GetStartT() const {

        return startXYT->GetT();
    }

    inline const Point3D* GetEndXYT() const {

        return endXYT;
    }

    inline double GetEndT() const {

        return endXYT->GetT();
    }

    inline Point2D GetStartWT() const {

        return Point2D(startW, startXYT->GetT());
    }

    inline Point2D GetEndWT() const {

        return Point2D(endW, endXYT->GetT());
    }

    inline double GetStartW() const {

        return startW;
    }

    inline double GetEndW() const {

        return endW;
    }

    inline double GetMaxW() const {

        return max(GetStartW(), GetEndW());
    }

    inline bool ResultFaceIsLeft() const {

        return resultFaceIsLeft;
    }

    inline bool ResultFaceIsRight() const {

        return !resultFaceIsLeft;
    }

    inline bool IsLeftBoundary() const {

        return isLeftBoundary;
    }

    inline bool IsRightBoundary() const {

        return !isLeftBoundary;
    }

    inline void MarkAsLeftBoundary() {

        isLeftBoundary = true;
    }

    inline void MarkAsRightBoundary() {

        isLeftBoundary = false;
    }

    inline void AddMate(IntersectionSegment* s) {

        // Note: We compare pointers here!
        if (matesOfThis->empty() || matesOfThis->back() != s) {
            
            matesOfThis->push_back(s);
            
        } else { // matesOfThis->back() == s
            
            // s is already a mate of this.
            // nothing to do...
        }
            
    }

    inline IntersectionSegment* GetActiveRightMate() const {

        if (this->IsLeftBoundary())
            return matesOfThis->front();
        else
            return matesOfBuddy->front();
    }

    inline TouchingMode GetRelationToRightBoundaryOfPFace() const {

        return relationToRightBoundary;
    }

    inline void SetRelationToRightBoundaryOfPFace(
            TouchingMode _relationToRightBoundary) {

        relationToRightBoundary = _relationToRightBoundary;
    }

    inline bool IsInserted() const {

        return *inserted;
    }

    inline void MarkAsInserted() {

        *inserted = true;
    }

    inline void Delete() {

        if (*hasReference) {

            *hasReference = false;
            delete this;

        } else {

            delete startXYT;
            delete endXYT;
            delete matesOfThis;
            delete matesOfBuddy;
            delete inserted;
            delete hasReference;
            delete this;
        }
    }

    inline bool IsOrthogonalToTAxis() const {

        return NumericUtil::NearlyEqual(GetStartT(), GetEndT());
    }
    /*
    
    inline bool IsLeftOf(const Point2D& pointWT) {
        
        return pointWT.IsLeft(GetStartWT(), GetEndWT());
    }
    
    inline bool IsRightOf(const Point2D& pointWT) {

        return pointWT.IsRight(GetStartWT(), GetEndWT());
    }
    
    */
    bool IsLeftOf(const IntersectionSegment* s) const;

    void SetWCoords(const PFace& pFace);

    void SetSideOfResultFace(const PFace& self, const PFace& other,
            const SetOp op);
    
    Point3D Evaluate(const double t) const;

    static pair<IntersectionSegment*, IntersectionSegment*> createBuddyPair(
            const Point3D& a, const Point3D& b);

    inline void SetStartWT(const Point2D& _startWT) {

        startW = _startWT.GetW();
    }

    inline void SetEndWT(const Point2D& _endWT) {

        endW = _endWT.GetW();
    }

    inline void SetStartW(const double _startW) {

        startW = _startW;
    }

    inline void SetEndW(const double _endW) {

        endW = _endW;
    }

    inline void SetResultFaceIsLeft() {

        resultFaceIsLeft = true;
    }

    inline void SetResultFaceIsRight() {

        resultFaceIsLeft = false;
    }
    
    inline void UpdateMateList(const double t) {
        
        assert(!matesOfThis->empty());
        assert(!matesOfBuddy->empty());

        if (NumericUtil::NearlyEqual(matesOfThis->front()->GetEndT(), t)) {

            matesOfThis->pop_front();
        }

        if (NumericUtil::NearlyEqual(matesOfBuddy->front()->GetEndT(), t)) {

            matesOfBuddy->pop_front();
        }
    }

    string GetVRMLDesc();
    void Print();
    void PrintMates();

private:

    Point3D* startXYT;
    Point3D* endXYT;
    deque<IntersectionSegment*>* matesOfThis;
    deque<IntersectionSegment*>* matesOfBuddy;
    bool* inserted;
    bool* hasReference;

    double startW;
    double endW;
    bool resultFaceIsLeft;
    bool isLeftBoundary;
    TouchingMode relationToRightBoundary;
    
    inline IntersectionSegment() {

        relationToRightBoundary = UNDEFINED;
    }
};

class ResultCycle {
    
    
};

class ResultUnit {
    
    inline void AddMSegment(const MSegmentData& mseg) {

        mSegments.push_back(mseg);
    }
    
    inline void SetInterval(const double start, const double end) {
        
        //interval = Interval<Instant>();
    }
    
private:
    
    vector<MSegmentData> mSegments;
    Interval<Instant> interval;
    //vector<ResultCycle> cycles;
};

struct GlobalIntSegSetCompare {

    bool operator()(const IntersectionSegment* const& s1,
            const IntersectionSegment* const& s2) const {

        return NumericUtil::Lower(s1->GetStartT(), s2->GetStartT());
    }
};

class ResultUnitFactory {

public:

    ResultUnitFactory(MRegion* const _resMRegion) :
        resMRegion(_resMRegion) {
        
    }

    void Start();

    inline void AddIntSeg(IntersectionSegment* intSeg) {

        if (!intSeg->IsInserted()) {

            source.insert(intSeg);
            intSeg->MarkAsInserted();
        }
    }

    void PrintIntSegsOfGlobalList();
    void PrintIntSegsOfGlobalListAsVRML(ofstream& target, const string& color);

private:

    void ComputeCurrentTimeLevel();
    void UpdateActiveSegList();

    inline bool IsOutOfRange(IntersectionSegment* seg) const {

        return NumericUtil::NearlyEqual(seg->GetEndT(), t1);
    }

    inline bool HasMoreSegsToInsert() const {

        return sourceIter != source.end() && 
        NumericUtil::NearlyEqual((*sourceIter)->GetStartT(), t1);
    }

    inline void UpdateMinEndT() {

        minEndT = min((*activeIter)->GetEndT(), minEndT);
    }

    multiset<IntersectionSegment*, GlobalIntSegSetCompare> source;
    multiset<IntersectionSegment*, GlobalIntSegSetCompare>::iterator sourceIter;

    list<IntersectionSegment*> active;
    list<IntersectionSegment*>::iterator activeIter;

    double t1;
    double t2;
    double minEndT;
    
    MRegion* const resMRegion;
};

class SourceUnit {

    friend class SourceUnitPair;

public:

    SourceUnit(const bool _isUnitA, 
               const URegionEmb* const _uRegion,
               const DBArray<MSegmentData>* const _array,
               SourceUnitPair* const _parent);

    inline void SetPartner(SourceUnit* _partner) {

        partner = _partner;
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

    inline Interval<Instant> GetTimeInterval() const {

        return uRegion->timeInterval;
    }

    inline Instant GetMedianTimeInstant() const {

        return medianTime;
    }

    inline Region GetMedianRegion() {

        if (!hasMedianRegion) {

            ComputeMedianRegion();
        }

        return medianRegion;
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

        return pFaceCountTotal;
    }

    void CreatePFaces();
    void AddBoundarySegments(const SetOp op);
    void FindMates();
    void CollectIntSegs(ResultUnitFactory* receiver);
    void FindMatesAndCollectIntSegs(ResultUnitFactory* receiver);

    bool IsEntirelyOutside(const PFace* pFace);

    void PrintPFaceCycles();
    void PrintPFacePairs();
    void PrintIntSegsOfPFaces();
    void PrintVRMLDesc(ofstream& target, const string& color);
    //void PrintMatesOfIntSegs();

private:

    void ComputeBoundingRect();
    //void ComputeMedianTime();
    void ComputeMedianRegion();

    const URegionEmb* const uRegion;
    const DBArray<MSegmentData>* const array;
    SourceUnit* partner;
    SourceUnitPair* const parent;
    const double startTime;
    const double endTime;
    Instant medianTime;
    list<PFace*> pFacesReduced;
    list<PFaceCycle> pFacesCycles;
    unsigned int cycleCount;
    unsigned int pFaceCountTotal;
    Rectangle<2> boundingRect;
    Region medianRegion;
    bool hasMedianRegion;
    const bool isUnitA;
};





class SourceUnitPair {

    friend class SourceUnit;

public:

    SourceUnitPair(const URegionEmb* const a, 
                   const DBArray<MSegmentData>* const aArray,
                   const URegionEmb* const b, 
                   const DBArray<MSegmentData>* const bArray,
                   MRegion* const resultMRegion);

    ~SourceUnitPair() {

    }

    void Operate(const SetOp op);

    inline Rectangle<2> GetOverlapRect() const {

        return overlapRect;
    }

    inline bool HasOverlappingBoundingRect() const {

        return overlapRect.IsDefined();
    }

    inline Interval<Instant> GetTimeInterval() const {

        return unitA.GetTimeInterval();
    }

    inline double GetStartTime() const {

        return unitA.GetStartTime();
    }

    inline double GetEndTime() const {

        return unitA.GetEndTime();
    }
    
private:

    void ComputeOverlapRect();
    void CreatePFaces();
    void ComputeInnerIntSegs(const SetOp op);
    void AddBoundarySegments(const SetOp op);
    void FindMatesAndCollectIntSegs(ResultUnitFactory* receiver);
    void ConstructResultUnits();

    void FindMates();
    void CollectIntSegs(ResultUnitFactory* receiver);

    void PrintPFaceCycles();
    void PrintPFacePairs();
    void PrintIntSegsOfPFaces();
    void PrintUnitsAsVRML();
    
    SourceUnit unitA;
    SourceUnit unitB;
    Rectangle<2> overlapRect;
    ResultUnitFactory resultUnitFactory;
};

class SetOperator {
    
public:
    
    SetOperator(MRegion* const _a, 
                MRegion* const _b, 
                MRegion* const _res) :
                
                a(_a), b(_b), res(_res) {

    }
    
    ~SetOperator() {
        
        // delete a_ref;
        // delete b_ref;
    }
    
    void Intersection();
    void Union();
    void Minus();
    
private:
    
    void RefinementPartition();
    void Operate(const SetOp op);
    
    MRegion* const a;
    MRegion* const b;
    MRegion* const res;
    
    MRegion* a_ref;
    MRegion* b_ref;
};

struct GeneralIntSegSetCompare {

    bool operator()(const IntersectionSegment* const& s1,
            const IntersectionSegment* const& s2) const;
};

struct RightIntSegSetCompare {

    bool operator()(const IntersectionSegment* const& s1,
            const IntersectionSegment* const& s2) const;
};

class MateEngine {

public:

    inline MateEngine(const set<IntersectionSegment*,
    GeneralIntSegSetCompare>* const _source) :

        source(_source) {
        
        sourceIter = source->begin();
        minEndT = MAX_DOUBLE;
    }
    
    void Start() {
        
        while (sourceIter != source->end()) {
            
            t = min((*sourceIter)->GetStartT(), minEndT);
            ComputeCurrentTimeLevel();
        }
    }
    
private:
    
    void ComputeCurrentTimeLevel();

    inline bool IsOutOfRange(IntersectionSegment* seg) const {
        
        return NumericUtil::NearlyEqual(seg->GetEndT(), t);
    }
    
    inline bool HasMoreSegsToInsert() const {

        return sourceIter != source->end() && NumericUtil::NearlyEqual(
                (*sourceIter)->GetStartT(), t);
    }
    
    inline void UpdateMinEndT() {
        
        minEndT = min((*activeIter)->GetEndT(), minEndT);
    }
    
    void DoMating();

    const set<IntersectionSegment*, GeneralIntSegSetCompare>* const source;
    list<IntersectionSegment*> active;
    
    list<IntersectionSegment*>::iterator activeIter;
    set<IntersectionSegment*, GeneralIntSegSetCompare>::const_iterator 
        sourceIter;
    
    double t;
    double minEndT;
};



class PFace {

    friend class PFacePair;

public:

    PFace(SourceUnit* _unit, 
          const MSegmentData* _mSeg, 
          bool _initialStartPointIsA);

    ~PFace() {

        DeleteGeneralIntSegs();
        DeleteOrthogonalIntSegs();
    }

    inline const MSegmentData* GetMSeg() const {

        return mSeg;
    }

    inline Rectangle<2> GetBoundingRect() const {

        return boundingRect;
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

    inline bool BelongsToOuterCycle() const {

        return mSeg->GetCycleNo() == 0;
    }

    inline bool InitialStartPointIsA() const {

        return initialStartPointIsA;
    }

    inline Vector3D GetWVector() const {

        return wVector;
    }

    inline Vector2D GetRightBoundaryVectorWT() const {

        return this->GetD_WT() - this->GetB_WT(); // vector BD
    }

    const IntersectionSegment* GetIntSegMaxW();

    bool Intersection(PFace& pf, const SetOp op);

    bool IntersectionPlaneSegment(const Segment3D& seg, Point3D& result);

    bool IsParallelTo(const PFace& pf) const;

    bool IntersectsRightBoundary(IntersectionSegment* intSeg) const;

    void AddInnerIntSeg(IntersectionSegment* intSeg);

    void AddBoundaryIntSegs(IntersectionSegment* intSeg);

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

    inline bool HasIntSegs() const {

        return hasIntSegs;

        // Alternativ:
        //return !(generalIntSegSet.empty() && orthogonalIntSegList.empty());
    }

    inline bool HasBoundaryIntSegs() const {

        return hasBoundaryIntSegs;
    }

    inline bool HasInnerIntSegs() const {

        return hasInnerIntSegs;
    }

    inline bool HasHorizontalIntSegs() const {

        return !orthogonalIntSegList.empty();
    }

    inline bool HasIntSegsTouchingRightBoundary() const {

        return !rightIntSegSet.empty();
    }

    inline void MarkAsEntirelyOutside() {

        entirelyInsideOutside = OUTSIDE;
    }

    inline bool IsEntirelyOutside() const {

        return entirelyInsideOutside == OUTSIDE;
    }

    inline void MarkAsEntirelyInside() {

        entirelyInsideOutside = INSIDE;
    }

    inline bool IsEntirelyInside() const {

        return entirelyInsideOutside == INSIDE;
    }

    inline bool IsPartOfUnitA() const {

        return unit->IsUnitA();
    }

    inline bool IsPartOfUnitB() const {

        return !unit->IsUnitA();
    }

    void FindMates();
    void CollectIntSegs(ResultUnitFactory* receiver) const;

    string GetVRMLDesc();

    void Print() const;
    void PrintGeneralIntSegs();
    void PrintRightIntSegs();
    void PrintOrthogonalIntSegs();

private:

    void ComputeBoundingRect();
    void ComputeNormalVector();
    void ComputeWTCoords();

    void DeleteOrthogonalIntSegs();
    void DeleteGeneralIntSegs();

    SourceUnit* const unit;
    const MSegmentData* const mSeg;
    Rectangle<2> boundingRect;
    Vector3D normalVector;
    Vector3D wVector;

    set<IntersectionSegment*, GeneralIntSegSetCompare> generalIntSegSet;
    set<IntersectionSegment*, RightIntSegSetCompare> rightIntSegSet;
    list<IntersectionSegment*> orthogonalIntSegList;
    //set<double> relevantTimeValueSet;

    //Point3D a, b, c, d;

    double aW;
    double bW;
    double cW;
    double dW;

    bool hasIntSegs;
    bool hasInnerIntSegs;
    bool hasBoundaryIntSegs;
    const bool initialStartPointIsA;
    InsideOutside entirelyInsideOutside;
    IntersectionSegment* intSegMaxW; // Does not make sense.

    //const unsigned int pFaceNoInCycle;

};

class PFacePair {

public:

    inline PFacePair(PFace* _left, PFace* _right) :
        left(_left), right(_right) {

    }

    inline PFace* GetLeft() const {

        return left;
    }

    inline PFace* GetRight() const {

        return right;
    }

    void ComputeBoundarySegments();

    inline void AddBoundarySegment(const Point3D& start, const Point3D& end) {

        pair<IntersectionSegment*, IntersectionSegment*> intSegPair =
                IntersectionSegment::createBuddyPair(start, end);

        (intSegPair.first)->SetResultFaceIsLeft();
        left->AddBoundaryIntSegs(intSegPair.first);

        (intSegPair.second)->SetResultFaceIsRight();
        right->AddBoundaryIntSegs(intSegPair.second);
    }

    inline void AddEntireBoundary() {

        AddBoundarySegment(left->GetB_XYT(), left->GetD_XYT());
    }

private:

    PFace* const left;
    PFace* const right;
};

class PFaceCycleIterator;

class PFaceCycle {

    friend class PFaceCycleIterator;

public:

    inline PFaceCycle(bool _isOuterCycle) :
        isOuterCycle(_isOuterCycle) {

    }

    inline ~PFaceCycle() {

        deque<PFace*>::iterator iter;
        for (iter = pFaces.begin(); iter != pFaces.end(); ++iter)
            delete *iter;
    }

    inline void Add(PFace* pFace) {

        if (pFace->InitialStartPointIsA())
            pFaces.push_back(pFace);
        else
            pFaces.push_front(pFace);
    }

    inline PFace* GetPFace(int i) {

        return pFaces[i];
    }

    inline unsigned int Size() {

        return pFaces.size();
    }

    PFaceCycleIterator GetPFaceCycleIterator();

private:

    deque<PFace*> pFaces;
    const bool isOuterCycle;
};

class PFaceCycleIterator {

    friend class PFaceCycle;

public:

    inline PFace* GetNext() {

        return *(iter++);
    }

    inline bool HasNext() {

        return iter != target->pFaces.end();
    }

    inline PFacePair GetNextPair() {

        iter++;

        if (iter != target->pFaces.end()) {

            iter--;
            PFace* left = *iter;
            iter++;
            PFace* right = *iter;

            return PFacePair(left, right);

        } else { // Last pair in cycle. 

            return PFacePair(target->pFaces.back(), target->pFaces.front());
        }
    }

    inline bool HasNextPair() const {

        return iter != target->pFaces.end();
    }

private:

    inline PFaceCycleIterator(PFaceCycle* pfc) :
        target(pfc) {

        iter = target->pFaces.begin();
    }

    PFaceCycle* const target;
    deque<PFace*>::iterator iter;
};

} // end of namespace mregionops


#endif /*SETOPUTIL_H_*/
