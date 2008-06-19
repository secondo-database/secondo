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
#include <vector>
#include <set>
#include <list>
#include <queue>
#include <deque>

namespace mregionops {

#define REDUCE_PFACES_BY_BOUNDING_RECT

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
class PUnit;
class PUnitPair;

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
        if (matesOfThis->empty() || matesOfThis->back() != s)
            matesOfThis->push_back(s);
    }

    inline IntersectionSegment* GetActiveLeftMate() const {

        if (this->IsRightBoundary())
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

class PUnit {

    friend class PUnitPair;

public:

    PUnit(const bool _isUnitA, const URegionEmb* const _uRegion,
            const DBArray<MSegmentData>* _array, PUnitPair* const _parent);

    inline void SetPartner(PUnit* _partner) {

        partner = _partner;
    }

    inline PUnitPair* GetParent() const {

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
    void CollectIntSegs(PUnitPair* target);
    void FindMatesAndCollectIntSegs(PUnitPair* target);

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
    PUnit* partner;
    PUnitPair* const parent;
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

struct GlobalIntSegSetCompare {

    bool operator()(const IntersectionSegment* const& s1,
            const IntersectionSegment* const& s2) const {

        return NumericUtil::Lower(s1->GetStartT(), s2->GetStartT());
    }
};

#define VRML_OUTFILE "out.wrl"

class PUnitPair {

    friend class PUnit;

public:

    PUnitPair(const URegionEmb* a, const DBArray<MSegmentData>* aArray,
            const URegionEmb* b, const DBArray<MSegmentData>* bArray);

    ~PUnitPair() {

        delete result;
    }

    vector<URegion>* Intersection();
    vector<URegion>* Union();
    vector<URegion>* Minus();

    inline void AddIntSeg(IntersectionSegment* intSeg) {

        if (!intSeg->IsInserted()) {

            globalIntSegSet.insert(intSeg);
            intSeg->MarkAsInserted();
        }
    }

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
    void FindMatesAndCollectIntSegs();
    void ConstructResultURegions();

    void FindMates();
    void CollectIntSegs();

    void PrintPFaceCycles();
    void PrintPFacePairs();
    void PrintIntSegsOfPFaces();
    void PrintIntSegsOfGlobalList();
    void PrintIntSegsOfGlobalListAsVRML(ofstream& target, const string& color);
    void PrintUnitsAsVRML();
    //void PrintMatesOfIntSegs();

    PUnit unitA;
    PUnit unitB;
    Rectangle<2> overlapRect;

    multiset<IntersectionSegment*, GlobalIntSegSetCompare> globalIntSegSet;
    vector<URegion>* result;
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

    inline MateEngine(set<IntersectionSegment*,
    GeneralIntSegSetCompare>* _source) :

        source(_source) {
        
        sourceIter = source->begin();
    }
    
    void ComputeTimeLevel(const double _t);

    inline bool IsOutOfRange(IntersectionSegment* seg) const {
        
        return NumericUtil::NearlyEqual(seg->GetEndT(), t);
    }
    
    inline bool HasMoreSegsToInsert() const {

        return sourceIter != source->end() && NumericUtil::NearlyEqual(
                (*sourceIter)->GetStartT(), t);
    }
    
    void DoMating();

private:

    set<IntersectionSegment*, GeneralIntSegSetCompare>* source;
    list<IntersectionSegment*> active;
    
    list<IntersectionSegment*>::iterator activeIter;
    set<IntersectionSegment*, GeneralIntSegSetCompare>::iterator sourceIter;
    
    double t;
};



class PFace {

    friend class PFacePair;

public:

    PFace(PUnit* _unit, const MSegmentData* _mSeg, bool _initialStartPointIsA);

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
    void CollectIntSegs(PUnitPair* target) const;

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

    PUnit* const unit;
    const MSegmentData* const mSeg;
    Rectangle<2> boundingRect;
    Vector3D normalVector;
    Vector3D wVector;

    set<IntersectionSegment*, GeneralIntSegSetCompare> generalIntSegSet;
    set<IntersectionSegment*, RightIntSegSetCompare> rightIntSegSet;
    list<IntersectionSegment*> orthogonalIntSegList;
    set<double> relevantTimeValueSet;

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
