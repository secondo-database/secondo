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

[1] Implementation of the Set Operator Classes

April - November 2008, M. H[oe]ger for bachelor thesis.

[TOC]

1 Introduction

This file essentially contains the implementations of several classes which 
provide the core functionality of the three set operators 
~intersection~, ~union~ and ~minus~ with the signature \\
movingregion [x] movingregion [->] movingregion \\
used in the MovingRegionAlgebra.

2 Defines and Includes

*/


#include "SetOps.h"

using namespace std;
using namespace datetime;

namespace temporalalgebra{

namespace mregionops {

set<unsigned int> flippedPFaces;
Statistic statistic;

/*
1 Class SetOperator

*/

void SetOperator::Intersection() {
    
    Operate(INTERSECTION);
}

void SetOperator::Union() {

    Operate(UNION);
}

void SetOperator::Minus() {

    Operate(MINUS);
}

void SetOperator::Operate(const SetOp op) {
    
    //flippedPFaces.clear();
    
    if (!a->IsDefined() || !b->IsDefined()) {
        
        res->SetDefined(false);
        return;
    }
    
    // Compute the RefinementPartition of the 
    // two MRegions:
    RefinementPartition<
    MRegion,
    MRegion,
    URegionEmb,
    URegionEmb> rp(*a, *b);
    
#ifdef PRINT_STATISTIC
    
    statistic.Reset();
    statistic.noUnitsIn = rp.Size();
    //cout << "RefinementPartition with " << rp.Size() << " units created.";
    //cout << endl;
    
#endif
    
    Interval<Instant> interval;
    int aPos;
    int bPos;
   
    bool aIsEmpty;
    bool bIsEmpty;
    
    MRegion* tempA;
    MRegion* tempB;
    
    URegionEmb unitA;
    URegionEmb unitB;
    
    URegionEmb unitARestrict;
    URegionEmb unitBRestrict;
    URegionEmb* unitARestrictCopy;
    URegionEmb* unitBRestrictCopy;
    const DbArray<MSegmentData>* aArray;
    const DbArray<MSegmentData>* bArray;
    
    SourceUnitPair* so;
    
    res->Clear();
    ((DbArray<MSegmentData>*)res->GetFLOB(1))->clean();
    res->StartBulkLoad();
    
    for (unsigned int i = 0; i < rp.Size(); i++) {
        
        // For each interval of the refinement partition...
        
        rp.Get(i, interval, aPos, bPos);
        
        Periods intervalAsPeriod(1);
        intervalAsPeriod.Add(interval);
        
        aIsEmpty = (aPos == -1);
        bIsEmpty = (bPos == -1);
        
        assert(!(aIsEmpty && bIsEmpty));
        
        if (aIsEmpty || bIsEmpty) {

            if (op == INTERSECTION) {

                // Result is empty: nothing to do.
                continue;
            }

            if (op == MINUS && aIsEmpty) {

                // Result is empty: nothing to do.
                continue;
            }
        }
        
        if (!aIsEmpty) {
                        
            a->Get(aPos, unitA);
            tempA = new MRegion(1);
            tempA->AtPeriods(&intervalAsPeriod, a);
            tempA->Get(0, unitARestrict);
            aArray = tempA->GetMSegmentData();
            
            unitARestrictCopy = new URegionEmb(unitARestrict.timeInterval,
                                    unitARestrict.GetStartPos());
            unitARestrictCopy->SetSegmentsNum(unitARestrict.GetSegmentsNum());
            unitARestrictCopy->SetBBox(unitARestrict.BoundingBox());
        }
        
        if (!bIsEmpty) {

            b->Get(bPos, unitB);
            tempB = new MRegion(1);
            tempB->AtPeriods(&intervalAsPeriod, b);
            tempB->Get(0, unitBRestrict);
            bArray = tempB->GetMSegmentData();
            
            unitBRestrictCopy = new URegionEmb(unitBRestrict.timeInterval,
                    unitBRestrict.GetStartPos());
            unitBRestrictCopy->SetSegmentsNum(unitBRestrict.GetSegmentsNum());
            unitBRestrictCopy->SetBBox(unitBRestrict.BoundingBox());
        }
 
        so = new SourceUnitPair(unitARestrictCopy, aArray, aIsEmpty,
                                unitBRestrictCopy, bArray, bIsEmpty,
                                op,
                                res);
        so->Operate();
        
        delete so;
        
        if (!aIsEmpty) {
            
            delete tempA;
            delete unitARestrictCopy;
        }
        
        if (!bIsEmpty) {
            
            delete tempB;
            delete unitBRestrictCopy;
        }
    }
             
    res->EndBulkLoad(false);

#ifdef PRINT_STATISTIC
    statistic.noUnitsResult = res->GetNoComponents();
    statistic.Print();
#endif
}

/*
1 Class SourceUnit

*/

SourceUnit::SourceUnit(const bool _isUnitA, 
             URegionEmb* const _uRegion,
             const DbArray<MSegmentData>* _array, 
             const bool _isEmpty,
             SourceUnitPair* const _parent) :
                 
            isUnitA(_isUnitA),
            isEmpty(_isEmpty),
            uRegion(_uRegion), 
            array(_array),
            parent(_parent), 
            hasNormalizedTimeInterval(false) {

    if (_isEmpty) 
        return;
    
    originalTimeInterval = _uRegion->timeInterval;
    
    NormalizeTimeInterval();
    ComputeBoundingRect();
    
    startTime = _uRegion->timeInterval.start.ToDouble();
    endTime = _uRegion->timeInterval.end.ToDouble();
}

SourceUnit::~SourceUnit() {
    
    vector<PFace*>::iterator iter;
    
    for (iter = pFaces.begin(); iter != pFaces.end(); iter++) {
        
        delete *iter;
    }
}

void SourceUnit::NormalizeTimeInterval() {
    
    if (!NORMALIZE_TIME_INTERVAL_OF_SOURCE_UNITS)
        return;
    
    //cout << "timeInterval.start: " << uRegion->timeInterval.start << endl;
    //cout << "timeInterval.end: " << uRegion->timeInterval.end << endl;
    
    //const double scaledDelta = 1.0;
    const double delta = 
        (originalTimeInterval.end - originalTimeInterval.start).ToDouble();
    
    //cout << "delta: " << delta << endl;
    
    if (delta >= 1.0) {
        
        hasNormalizedTimeInterval = false;
        return;
    }
    
    hasNormalizedTimeInterval = true;
    uRegion->timeInterval.start.SetToZero();
    uRegion->timeInterval.end.ReadFrom(1.0);
    
    // Adjust the bounding box:
    const double minX = uRegion->BoundingBox().MinD(0);
    const double maxX = uRegion->BoundingBox().MaxD(0);
    const double minY = uRegion->BoundingBox().MinD(1);
    const double maxY = uRegion->BoundingBox().MaxD(1);
    const double minT = 0.0;
    const double maxT = 1.0;
    double minMax[] = { minX, maxX, minY, maxY, minT, maxT};
    uRegion->SetBBox(Rectangle<3>(true,minMax ));
    
    //scaleFactor = delta / scaledDelta;
    //scaleFactor = delta;
    
    //cout << "scaleFactor: " << scaleFactor << endl;
    //cout << "timeInterval.start: " << uRegion->timeInterval.start << endl;
    //cout << "timeInterval.end: " << uRegion->timeInterval.end << endl;
}

void SourceUnit::ComputeBoundingRect() {

    // Calculate the projection bounding rectangle in the (x, y)-plane
    // of the URegion:
    
    const double minX = uRegion->BoundingBox().MinD(0);
    const double maxX = uRegion->BoundingBox().MaxD(0);
    const double minY = uRegion->BoundingBox().MinD(1);
    const double maxY = uRegion->BoundingBox().MaxD(1);
    double minMax[] = { minX, maxX, minY, maxY};
    boundingRect = Rectangle<2>(true,minMax );
}

void SourceUnit::CreatePFaces() {

    MSegmentData segment;
    
    for (int i = 0; i < uRegion->GetSegmentsNum(); i++) {
        
        uRegion->GetSegment(array, i, segment);
        PFace* pFace = new PFace(this, &segment);
        pFaces.push_back(pFace);

#ifdef OPTIMIZE_BY_BOUNDING_RECT
        
        if (parent->HasOverlappingBoundingRect()) {

            if (pFace->GetBoundingRect().Intersects(parent->GetOverlapRect()))
                pFacesReduced.push_back(pFace);
            else
                pFace->MarkAsEntirelyOutside();

        } else {

            pFace->MarkAsEntirelyOutside();
            pFacesReduced.push_back(pFace);
        }

#else
        pFacesReduced.push_back(pFace);
#endif

    }
    
    statistic.noPFaceTotal += pFaces.size();
    statistic.noPFaceReducedByPBR += pFacesReduced.size();
}

void SourceUnit::CollectRelevantPFaces(ResultUnitFactory* receiver) {

    vector<PFace*>::iterator iter;
    
    for (iter = pFaces.begin(); iter != pFaces.end(); iter++) {
        
        (*iter)->Finalize();
        
        assert((*iter)->GetState() != UNKNOWN);
        assert((*iter)->GetState() != ENTIRELY_INSIDE);
        assert((*iter)->GetState() != ENTIRELY_OUTSIDE);
        
        if ((*iter)->IsNormalRelevant() || (*iter)->IsCriticalRelevant()) {
            
            receiver->AddPFace(*iter);
        }
    }
}

void SourceUnit::PrintPFaces() {
    
    for (vector<PFace*>::iterator iter = pFaces.begin(); 
                iter != pFaces.end(); iter++) {
        
        (*iter)->Print();
    }
}

void SourceUnit::Print() const {
    
    cout << ((IsUnitA()) ? "Unit A:" : "Unit B:") << endl;
    cout << "Interval: " << GetTimeInterval() << endl;
    cout << "Interval as double: " << GetTimeInterval().start.ToDouble() 
         << " -> " << GetTimeInterval().end.ToDouble() << endl;
    cout << "No of PFaces total: " << pFaces.size() << endl;
    cout << "No of PFaces reduced: " << pFacesReduced.size() << endl;
}

void SourceUnit::PrintVRMLDesc(ofstream& target, const string& color) {

    const double scale = VRML_SCALE_FACTOR;

    target << "Transform {" << endl;
    target << "\tscale " << scale << " " << scale << " " << scale << endl;
    target << "\tchildren [" << endl;
    target << "\t\tShape {" << endl;
    target << "\t\t\tgeometry IndexedLineSet {" << endl;
    target << "\t\t\t\tcoord Coordinate {" << endl;
    target << "\t\t\t\t\tpoint [" << endl;

    int pFaceCount = 0;

    for (vector<PFace*>::iterator iter = pFaces.begin(); 
            iter != pFaces.end(); iter++) {

        target << "\t\t\t\t\t\t" << (*iter)->GetVRMLDesc() << endl;
        pFaceCount++;
    }

    target << "\t\t\t\t\t]  # end point" << endl;
    target << "\t\t\t\t} # end coord" << endl;
    target << "\t\t\t\tcoordIndex [" << endl;

    const int noPoints = pFaceCount * 4;

    for (int i = 0; i < noPoints; i += 4) {

        target << "\t\t\t\t\t" << i << ", " << i + 1 << ", " << i + 2 << ", "
                << i + 3 << ", " << i << ", " << "-1," << endl;
    }

    target << "\t\t\t\t] # end coordIndex" << endl;

    target << "\t\t\t\tcolor Color { color [ " << color << " ] }" << endl;

    target << "\t\t\t} # end geometry" << endl;
    target << "\t\t} # end shape" << endl;
    target << "\t] # end children" << endl;
    target << "} # end Transform" << endl;
}

bool SourceUnit::IsEntirelyOutside(const PFace* pFace) {
    
    // Precondition: pFace is known as entirely inside or entirely outside!

    // Check, if the answer is already known:

    if (pFace->IsEntirelyOutside())
        return true;
    
    if (pFace->IsEntirelyInside())
        return false;

    // We don't know and have to use the plumbline algorithm
    // to find the answer:
    
    assert(pFace->GetState() == UNKNOWN);
    
    return IsOutside(pFace->GetMidPoint());
}

bool SourceUnit::IsOutside(const Point3D& p) {
    
    const Point p2D(true, p.GetX(), p.GetY());
    
    return !GetTestRegion(p.GetT()).Contains(p2D);
}

bool SourceUnit::IsOnBorder(const Point3D& p) {

    const Point p2D(true, p.GetX(), p.GetY());

    if (GetTestRegion(p.GetT()).OnBorder(p2D)) {

        return true;
    }

    // This is not very nice, maybe sometimes even wrong...
    // It is necessary, since Region::OnBorder may
    // return false-negative results, because of rounding errors.

    // We paint a 'small' triangle around p and check,
    // if it intersects the testregion's border.
    const double d = NumericUtil::eps * 100;

    const Point p1(true, p.GetX() - d, p.GetY() - d);
    const Point p2(true, p.GetX() + d, p.GetY() - d);
    const Point p3(true, p.GetX(), p.GetY() + d);

    const bool p1IsInside = GetTestRegion(p.GetT()).Contains(p1);
    const bool p2IsInside = GetTestRegion(p.GetT()).Contains(p2);
    const bool p3IsInside = GetTestRegion(p.GetT()).Contains(p3);

    const int sum = p1IsInside + p2IsInside + p3IsInside;

    if (sum == 0 || sum == 3) {

        // The triangle is entirely outside or inside the testregion:
        return false;

    } else { // sum == 1 || sum == 2

        // The triangle intersects the testregion's border:
        return true;
    }
}

#ifdef CACHE_TESTREGIONS

const Region SourceUnit::GetTestRegion(const double t) {
    
    Instant instant(instanttype);
    instant.ReadFrom(t);
    
    if (!testRegion.defined || testRegion.instant != instant) {
        
        //cout << "Create new TestRegion at instant: " << instant << endl;
        statistic.NoTestRegionsCreated++;
        
        Region newTestRegion(0);
        GetURegionEmb()->TemporalFunction(array, instant, newTestRegion);
        testRegion.instant = instant;
        testRegion.region = newTestRegion;
        testRegion.defined = true;
        
    } else {
        
        //cout << "TestRegion cachehit at instant: " << instant << endl;
        statistic.NoTestRegionsCacheHits++;
    }
        
    return testRegion.region;
}

#else

const Region SourceUnit::GetTestRegion(const double t) {
    
    Instant instant(instanttype);
    instant.ReadFrom(t);
    
    Region testRegion(0);
    GetURegionEmb()->TemporalFunction(array, instant, testRegion);
    
    return testRegion;
}

#endif

const SetOp SourceUnit::GetOperation() const {

    return parent->GetOperation();
}

void SourceUnit::AddGlobalTimeValue(double t) {
    
    parent->AddGlobalTimeValue(t);
}

void SourceUnit::AddToMRegion(MRegion* const target) const {
    
    if (IsEmpty())
        return;
    
    DbArray<MSegmentData>* targetArray = 
        (DbArray<MSegmentData>*)target->GetFLOB(1);
    const int segmentsStartPos = targetArray->Size();
    
    URegionEmb targetUnit(GetOriginalTimeInterval(), segmentsStartPos);
    
    MSegmentData segment;
        
    for (int i = 0; i < uRegion->GetSegmentsNum(); i++) {

        uRegion->GetSegment(array, i, segment);
        targetUnit.PutSegment(targetArray, i, segment, true);
    }
    
    target->Add(targetUnit);
}

/*
1 Class ResultUnitFactory

*/

void ResultUnitFactory::Start() {
    
    if (time.size() < 2) {
        
        // Result is empty.
        return;
    }
    
    // Set the first initial timelevel:
    timeIter = time.begin();
    t1 = *timeIter;

    while ((++timeIter) != time.end()) {

        // Set the final timelevel:
        t2 = *timeIter;
        
        // Set the median timelevel:
        t12 = (t1 + t2) / 2.0;

        ComputeCurrentTimeLevel();
      
        // Set the new initial timelevel:
        t1 = t2;
    }
}

void ResultUnitFactory::ComputeCurrentTimeLevel() {
    
    //cout << "t1: " << t1 << endl;
    //cout << "t12: " << t12 << endl;
    //cout << "t2: " << t2 << endl;
        
    assert(NumericUtil::Lower(t1, t2));
    
    Instant starttime(instanttype);
    Instant endtime(instanttype);
    
    if (parent->HasNormalizedTimeInterval()) {
        
        const double originalStart = 
            parent->GetOriginalTimeInterval().start.ToDouble();
        
        const double originalEnd = 
            parent->GetOriginalTimeInterval().end.ToDouble();
        
        starttime.ReadFrom((1.0 - t1) * originalStart + t1 * originalEnd);
        endtime.ReadFrom((1.0 - t2) * originalStart + t2 * originalEnd);
        
    } else {
        
        starttime.ReadFrom(t1);
        endtime.ReadFrom(t2);
    }
    
    const Interval<Instant> interval(starttime, endtime, true, false);

    //cout << "start: " << starttime.ToDouble() << endl;
    //cout << "end: " << endtime.ToDouble() << endl;

    resultUnit = new ResultUnit(interval);
    resultUnit->StartBulkLoad();

    vector<PFace*>::iterator iter;
    for (iter = pFaces.begin(); iter != pFaces.end(); iter++) {

        PFace* pFace = *iter;
        //pFace->Print();
        pFace->UpdateTimeLevel(t1);

        if (pFace->IsNormalRelevant()) {

            ProcessNormalPFace(pFace);

        } else { // pFace->IsCriticalRelevant()

            ProcessCriticalPFace(pFace);
        }
    }

    //cout << "ResultUnit: " << t1 << " -> " << t2 << endl;
    //Instant mediantime(instanttype);
    //mediantime.ReadFrom(t12);
    //cout << "mediantime: " << mediantime.ToString() << endl;

    AddCriticalMSegments();

    //cout << "ResultUnit: " << t1 << " -> " << t2 << endl;

    //assert(false);

    //cout << endl;
    //cout << "ResultUnit: " << t1 << " -> " << t2 << endl;
    //cout << endl;

    //resultUnit->WriteToVRMLFile();
    //this->Print();

    resultUnit->EndBulkLoad(MERGE_RESULT_MSEGMENTS);

    ConstructResultUnitAsURegionEmb();

    delete resultUnit;
}

void ResultUnitFactory::ProcessNormalPFace(PFace* pFace) {
    
    //assert(NumericUtil::Lower(t1, t2));
    
#ifdef PRINT_STATISTIC
        
        StopWatch stopWatch;
        stopWatch.start();
#endif

    const list<IntersectionSegment*>* intSegs;
    list<IntersectionSegment*>::const_iterator intSegIter;
    
    Decision decision;
    Decision lastDecision = pFace->GetLastDecision();
    bool firstMSegInTimeLevel = true;

    intSegs = pFace->GetActiveIntSegs();
    intSegIter = intSegs->begin();

    assert(intSegs->size() >= 2);

    const IntersectionSegment* start = *intSegIter;
    const IntersectionSegment* end;

    Point3D initialStart3D(start->Evaluate(t1));
    Point3D mediumStart3D(start->Evaluate(t12));
    Point3D finalStart3D(start->Evaluate(t2));

    while ((++intSegIter) != intSegs->end()) {

        end = *intSegIter;
        
        evalutedIntSegs++;

        Point3D initialEnd3D(end->Evaluate(t1));
        Point3D mediumEnd3D(end->Evaluate(t12));
        Point3D finalEnd3D(end->Evaluate(t2));
        
        Segment3D initial(initialStart3D, initialEnd3D);
        Segment3D medium(mediumStart3D, mediumEnd3D);
        Segment3D final(finalStart3D, finalEnd3D);
        
        if (mediumStart3D == mediumEnd3D) {

            decisionsByDegeneration++;
            
            cout << "Warning: Degenerated MSegment in normal PFace found!" 
            << endl;
            cout << "This should never happen..." << endl;
            cout << "initial: " << initial << endl;
            cout << "medium: " << medium << endl;
            cout << "final" << final << endl;
            
            start = end;
            continue;
        }

            if (lastDecision == UNDEFINED || DECIDE_BY_PLUMBLINE_ONLY) {

                Point3D midpoint3D((mediumStart3D + mediumEnd3D) * 0.5);
                decision = BelongsToResultUnit(midpoint3D, pFace);

                if (firstMSegInTimeLevel) {

                    pFace->SetLastDecision(decision);
                    firstMSegInTimeLevel = false;
                }

            } else { // lastDecision != UNDEFINED

                decisionsByAdjacency++;

                if (firstMSegInTimeLevel) {

                    decision = lastDecision;
                    pFace->SetLastDecision(decision);
                    firstMSegInTimeLevel = false;

                } else {

                    if (lastDecision == ACCEPT)
                        decision = SKIP;
                    else
                        decision = ACCEPT;
                }
            }
        
        
        switch (decision) {

        case SKIP:

            // This msegment does not contribute to the result.
            noMSegsSkipped++;
            break;

        case ACCEPT:

            // This msegment contributes to the result.
            resultUnit->AddSegment(MSegment(initial, medium, final, pFace));
            noMSegsValid++;
            break;
            
        case UNDEFINED:
            
            // This case should never occur:
            assert(false);
            break;
        }
        
        start = end;
        initialStart3D = initialEnd3D;
        mediumStart3D = mediumEnd3D;
        finalStart3D = finalEnd3D;
        lastDecision = decision;
    }
    
#ifdef PRINT_STATISTIC
    statistic.durationProcessNormalPFace += stopWatch.diffSecondsReal();
#endif
}

void ResultUnitFactory::ProcessCriticalPFace(PFace* pFace) {
    
    //assert(NumericUtil::Lower(t1, t2));
    
#ifdef PRINT_STATISTIC
        
        StopWatch stopWatch;
        stopWatch.start();
#endif
    
    const list<IntersectionSegment*>* intSegs;
    list<IntersectionSegment*>::const_iterator intSegIter;

    Decision decision;

    intSegs = pFace->GetActiveIntSegs();
    intSegIter = intSegs->begin();

    assert(intSegs->size() >= 2);

    const IntersectionSegment* start = *intSegIter;
    const IntersectionSegment* end;

    Point3D initialStart3D(start->Evaluate(t1));
    Point3D mediumStart3D(start->Evaluate(t12));
    Point3D finalStart3D(start->Evaluate(t2));

    while ((++intSegIter) != intSegs->end()) {

        end = *intSegIter;
        
        evalutedIntSegs++;

        Point3D initialEnd3D(end->Evaluate(t1));
        Point3D mediumEnd3D(end->Evaluate(t12));
        Point3D finalEnd3D(end->Evaluate(t2));
        
        Point3D midpoint3D((mediumStart3D + mediumEnd3D) * 0.5);
        
        Segment3D initial(initialStart3D, initialEnd3D);
        Segment3D medium(mediumStart3D, mediumEnd3D);
        Segment3D final(finalStart3D, finalEnd3D);
        
        //cout << "initial: " << initial << endl;
        //cout << "medium:  " << medium << endl;
        //cout << "final:   " << final << endl;
        //cout << "midpoint3D: " << midpoint3D << endl;

        if (mediumStart3D == mediumEnd3D) {

            decisionsByDegeneration++;
            decision = SKIP;
            
        } else {
            
            decision = BelongsToResultUnit(midpoint3D, pFace);
        }
        
        switch (decision) {

        case SKIP:

            // This msegment does not contribute to the result.
            noMSegsSkipped++;
            //cout << "decision: SKIP" << endl;
            break;

        case ACCEPT:

            // This msegment contributes to the result.
            resultUnit->AddSegment(MSegment(initial, medium, final, pFace));
            noMSegsValid++;
            //cout << "decision: ACCEPT" << endl;
            break;

        case UNDEFINED:

            // This msegment is part of a pair of identical ones
            // and we can't do a decision now.
            criticalMSegs.push_back(MSegmentCritical(initial, medium, final,
                                                     midpoint3D, pFace));
            noMSegCritical++;
            //cout << "decision: UNDEFINED" << endl;
            break;
        }
        
        //cout << endl;

        start = end;
        initialStart3D = initialEnd3D;
        mediumStart3D = mediumEnd3D;
        finalStart3D = finalEnd3D;
    }
    
#ifdef PRINT_STATISTIC
    statistic.durationProcessCriticalPFace += stopWatch.diffSecondsReal();
#endif
}

Decision ResultUnitFactory::BelongsToResultUnit(const Point3D& midpoint, 
                                                const PFace* pFace) {
    
    if (DECIDE_BY_ENTIRELY_IN_OUT && 
        pFace->IsNormalRelevant() && 
        !pFace->HasInnerIntSegs()) {
        
        decisionsByEntirelyInOut++;
        return ACCEPT;
    }
    
    const SourceUnit* ownUnit = pFace->GetUnit();
    SourceUnit* otherUnit = ownUnit->GetPartner();
    const SetOp op = parent->GetOperation();
    
    decisionsByPlumbline++;
    
    if (pFace->IsCriticalRelevant() && otherUnit->IsOnBorder(midpoint)) {
        
        // The testpoint is on the border of the testregion.
        // Hence we can decide it not yet:
        return UNDEFINED;
    }
    
    if (otherUnit->IsOutside(midpoint)) {
        
        if (op == UNION || (op == MINUS && ownUnit->IsUnitA()))
            return ACCEPT;
        else
            return SKIP;
    } 
    
    // The Point is inside of otherUnit.

    if (op == INTERSECTION || (op == MINUS && ownUnit->IsUnitB()))
        return ACCEPT;
    else
        return SKIP;
}

void ResultUnitFactory::AddCriticalMSegments() {
    
    if (criticalMSegs.size() == 0)
        return;
    
    // Sort by midpoints:
    sort(criticalMSegs.begin(), criticalMSegs.end());
    
    const SetOp op = parent->GetOperation();
    
    //cout << "criticalMSegs.size() = " << criticalMSegs.size() << endl;
    
    
    //for (unsigned int i = 0; i < criticalMSegs.size(); i++) {
        
        //criticalMSegs[i].Print();
        //cout << "midpoint: ";
        //cout << criticalMSegs[i].GetMidpoint().GetX() << " | " <<
        //criticalMSegs[i].GetMidpoint().GetY() << endl;
    //}
    
    assert(criticalMSegs.size() % 2 == 0);
    
    for (unsigned int i = 0; i < criticalMSegs.size(); i += 2) {
        
        MSegmentCritical mSeg1 = criticalMSegs[i];
        MSegmentCritical mSeg2 = criticalMSegs[i + 1];
        
        //cout << "mSeg1.GetMidpoint(): " << mSeg1.GetMidpoint() << endl;
        //cout << "mSeg2.GetMidpoint(): " << mSeg2.GetMidpoint() << endl;
        assert(mSeg1.GetMidpoint() == mSeg2.GetMidpoint());

        if (op == UNION || op == INTERSECTION) {

            if (!mSeg1.HasEqualNormalVector(mSeg2)) {

                // Skip both.
                noMSegsSkipped++;
                noMSegsSkipped++;

            } else { // HasEqualNormalVector(other)

                if (mSeg1.IsPartOfUnitA())
                    resultUnit->AddSegment(mSeg1);
                else
                    resultUnit->AddSegment(mSeg2);
                
                noMSegsValid++;
                noMSegsSkipped++;
            }

        } else { // op == MINUS

            if (mSeg1.HasEqualNormalVector(mSeg2)) {

                // Skip both.
                noMSegsSkipped++;
                noMSegsSkipped++;

            } else { // !HasEqualNormalVector(other)

                if (mSeg1.IsPartOfUnitA())
                    resultUnit->AddSegment(mSeg1);
                else
                    resultUnit->AddSegment(mSeg2);
                
                noMSegsValid++;
                noMSegsSkipped++;
            }
        }
    }
    
    criticalMSegs.clear();
}

void ResultUnitFactory::ConstructResultUnitAsURegionEmb() {
    
    if (resultUnit->IsEmpty()) {
        
        noEmptyUnits++;
        return;
    }
    
    noUnits++;
    
    //cout <<  "ResultUnit created: " << resultUnit->GetInterval() << endl;
    //cout << endl;

#ifdef  WRITE_VRML_FILE
    
    vrml.push_back(resultUnit->GetVRMLDesc());
    
#endif
    
    const Interval<Instant> interval = resultUnit->GetInterval();
    DbArray<MSegmentData>* array = 
        (DbArray<MSegmentData>*)resMRegion->GetFLOB(1);
    
    URegionEmb* ure = resultUnit->ConvertToURegionEmb(array);
    //cout << NList(resultUnit->ConvertToListExpr()) << endl;
    resMRegion->Add(*ure);
    delete ure;
}

void ResultUnitFactory::Print() const {
    
    cout <<  "ResultUnitFactory: " << endl;
    cout <<  "Units created: " << noUnits << endl;
    cout <<  "Empty units: " << noEmptyUnits << endl;
    cout <<  "Processed PFaces total: " << pFaces.size() << endl;
    cout <<  "MSegments total: " << evalutedIntSegs << endl;
    cout <<  "MSegments valid: " << noMSegsValid << endl;
    cout <<  "MSegments skipped: " << noMSegsSkipped << endl;
    cout <<  "MSegments critical: " << noMSegCritical << endl;
    cout <<  "Decisions by plumbline: " << decisionsByPlumbline << endl;
    cout <<  "Decisions by entirely inside/outside: " <<  
                decisionsByEntirelyInOut << endl;
    cout <<  "Decisions by adjacency: " << decisionsByAdjacency << endl;
    cout <<  "Decisions by degeneration: " << decisionsByDegeneration << endl;
    cout << endl;
}

string ResultUnitFactory::GetVRMLDesc() const {
    
    std::ostringstream oss;
    
    for (unsigned int i = 0; i < vrml.size(); i++) {
        
        oss << vrml[i] << endl;
    }
    
    return oss.str();
}

void ResultUnitFactory::AddToOverallStatistic() const {
    
    statistic.noRelevantPFaces += pFaces.size();
    statistic.noMSegsOverall += evalutedIntSegs;
    statistic.noMSegsValidOverall += noMSegsValid;
    statistic.noMSegsSkippedOverall += noMSegsSkipped;
    statistic.noMSegCriticalOverall += noMSegCritical;
    statistic.decisionsByPlumblineOverall += decisionsByPlumbline;
    statistic.decisionsByEntirelyInOutOverall += decisionsByEntirelyInOut;
    statistic.decisionsByAdjacencyOverall += decisionsByAdjacency;
    statistic.decisionsByDegenerationOverall += decisionsByDegeneration;
}

/*
1 Class SourceUnitPair

*/

SourceUnitPair::SourceUnitPair(URegionEmb* const _unitA,
                               const DbArray<MSegmentData>* const _aArray, 
                               const bool _aIsEmpty,
                               URegionEmb* const _unitB,
                               const DbArray<MSegmentData>* const _bArray,
                               const bool _bIsEmpty,
                               const SetOp _operation,
                               MRegion* const _resultMRegion) :
                                   
    unitA(true, _unitA, _aArray, _aIsEmpty, this),
    unitB(false, _unitB, _bArray, _bIsEmpty, this),
    op(_operation),
    resultMRegion(_resultMRegion),
    resultUnitFactory(_resultMRegion, this) {
    
    if (_aIsEmpty || _bIsEmpty)
        return;
    
    assert(_unitA->timeInterval == _unitB->timeInterval);
    
    unitA.SetPartner(&unitB);
    unitB.SetPartner(&unitA);

    ComputeOverlapRect();
    
    //cout << "SourceUnitPair with interval " << GetOriginalTimeInterval() 
         //<< " created." << endl;
    
#ifdef PRINT_DEBUG_MESSAGES
    
    if (HasNormalizedTimeInterval())
        cout << "Interval was normalized to: " << GetTimeInterval()
             << endl;
    
#endif
}

void SourceUnitPair::Operate() {
    
#ifdef OPTIMIZE_BY_BOUNDING_RECT

    const bool s = false;
    
#else
    
    const bool s = true;
        
#endif

    if ((!unitA.IsEmpty() && !unitB.IsEmpty()) && 
       (s || op == UNION || HasOverlappingBoundingRect())) {

#ifdef  PRINT_STATISTIC
        
        StopWatch stopWatch;
        stopWatch.start();
        
#endif
        
        CreatePFaces();
        
#ifdef  PRINT_STATISTIC
        
        statistic.durationCreatePFacesOverall += stopWatch.diffSecondsReal();
#endif
#ifdef  PRINT_DEBUG_MESSAGES      
        
        cout << "*************************************************************";
        cout << endl;
        cout << "PFaces after calling CreatePFaces():" << endl;
        cout << endl;
        PrintPFaces();
        
#endif
#ifdef  PRINT_STATISTIC  
        
        stopWatch.start();
        
#endif
        
        ComputeIntSegs();
        
#ifdef  PRINT_STATISTIC
        
        statistic.durationComputeIntSegsOverall += stopWatch.diffSecondsReal();
#endif
#ifdef  PRINT_DEBUG_MESSAGES
        
        cout << "*************************************************************";
        cout << endl;
        cout << "PFaces after calling ComputeIntSegs():" << endl;
        cout << endl;
        PrintPFaces();
        
#endif
#ifdef  PRINT_STATISTIC
        
        stopWatch.start();
        
#endif       
        
        CollectRelevantPFaces();
        
#ifdef  PRINT_STATISTIC
        
        statistic.durationCollectRelevantPFacesOverall
                += stopWatch.diffSecondsReal();
#endif
#ifdef  PRINT_DEBUG_MESSAGES     
        
        cout << "*************************************************************";
        cout << endl;
        cout << "PFaces after calling CollectRelevantPFaces():" << endl;
        cout << endl;
        PrintPFaces();
        
#endif
#ifdef  PRINT_STATISTIC
        
        stopWatch.start();
        
#endif
        
        ConstructResultUnits();
        
#ifdef  PRINT_STATISTIC
        
        statistic.durationConstructResultUnitsOverall
                += stopWatch.diffSecondsReal();
        
        resultUnitFactory.AddToOverallStatistic();
#endif
#ifdef  WRITE_VRML_FILE
        
        ToVrmlFile(showSourceUnitA, showSourceUnitB, showResultUnits);
        
#endif

    } else {
        
        // unitA.IsEmpty() || unitB.IsEmpty() || 
        // (!s && op != UNION && !HasOverlappingBoundingRect())

        switch (op) {
        
        case MINUS:
            
            // Result is unit a:
            unitA.AddToMRegion(resultMRegion);
            break;

        case INTERSECTION:

            // Result is empty:
            // Nothing to do.
            break;
            
        case UNION:
            
            assert(unitA.IsEmpty() || unitB.IsEmpty());
            
            unitA.AddToMRegion(resultMRegion);
            unitB.AddToMRegion(resultMRegion);
            break;
        }
    }
}

void SourceUnitPair::ComputeOverlapRect() {

    overlapRect = unitA.GetBoundingRect().Intersection(unitB.GetBoundingRect());
}

void SourceUnitPair::CreatePFaces() {

    unitA.CreatePFaces();
    unitB.CreatePFaces();
}

void SourceUnitPair::ComputeIntSegs() {
    
    vector<PFace*>::iterator iterA;
    vector<PFace*>::iterator iterB;

    for (iterA = unitA.pFacesReduced.begin(); iterA
                            != unitA.pFacesReduced.end(); iterA++) {

        for (iterB = unitB.pFacesReduced.begin(); iterB
                                != unitB.pFacesReduced.end(); iterB++) {

            (*iterA)->Intersection(**iterB);
        }
    }
}

void SourceUnitPair::CollectRelevantPFaces() {
    
    unitA.CollectRelevantPFaces(&resultUnitFactory);
    unitB.CollectRelevantPFaces(&resultUnitFactory);
}

void SourceUnitPair::ConstructResultUnits() {
    
    resultUnitFactory.Start();
}

void SourceUnitPair::PrintPFaces() {

    cout << endl;
    cout << "*********************************************" << endl;
    cout << "PFaces of Unit A:" << endl;
    cout << "*********************************************" << endl;
    cout << endl;

    unitA.PrintPFaces();

    cout << endl;
    cout << "*********************************************" << endl;
    cout << "PFaces of Unit B:" << endl;
    cout << "*********************************************" << endl;
    cout << endl;

    unitB.PrintPFaces();

}

void SourceUnitPair::ToVrmlFile(bool a, bool b, bool res) {
    
    if (!a && !b && !res)
        return;

    const string colorA = "1 1 0";
    const string colorB = "0 0.6 1";
    //const string colorResult = "1 0 0";

    const string filename = "unitpair_" + 
                            GetTimeInterval().start.ToString() + 
                            ".wrl";
    
    ofstream target(filename.c_str());

    if (!target.is_open()) {

        cerr << "Unable to open file: " << filename << endl;
        return;
    }

    target << "#VRML V2.0 utf8" << endl;

    if (a) {

        target << endl;
        target << "# Unit A:" << endl;
        target << endl;
        unitA.PrintVRMLDesc(target, colorA);

    }

    if (b) {

        target << endl;
        target << "# Unit B:" << endl;
        target << endl;
        unitB.PrintVRMLDesc(target, colorB);

    }

    if (res) {

        target << endl;
        target << "# Result units:" << endl;
        target << endl;
        target << resultUnitFactory.GetVRMLDesc() << endl;
    }
    
    target.close();
}

/*
1 Class IntersectionSegment

*/

unsigned int IntersectionSegment::instanceCount = 0;

IntersectionSegment::IntersectionSegment(const Segment3D& s) :

    id(instanceCount++),
    pFace(0) {

    // The startpoint's t-coord is always lower or equal to the
    // endpoint's t-coord.
    // Note: We don't care for x and y!

    if (s.GetStart().GetT() <= s.GetEnd().GetT()) {

        startXYT = s.GetStart();
        endXYT = s.GetEnd();

    } else {

        startXYT = s.GetEnd();
        endXYT = s.GetStart();
    }
    
    statistic.noIntSegsTotal++;
}

bool IntersectionSegment::IsLeftOf(const IntersectionSegment* intSeg) const {
    
    // Precondition: 
    // this->GetStartT() is inside the interval 
    // [intSeg->GetStartT(), intSeg->GetEndT()]
    // and
    // this and intSeg don't intersect in their interior.
    
    assert(NumericUtil::GreaterOrNearlyEqual(this->GetStartT(), 
                                             intSeg->GetStartT())
           && 
           
           NumericUtil::LowerOrNearlyEqual(this->GetStartT(), 
                                           intSeg->GetEndT()));
    
    //const double eps = NumericUtil::eps * 100;
    const double eps = NumericUtil::eps;
    
    const double sideOfStart = GetStartWT()->WhichSide(*intSeg->GetStartWT(), 
                                                       *intSeg->GetEndWT());
    
    if (sideOfStart > eps)
        return true;
    
    if (sideOfStart < -eps)
        return false;
    
    const double sideOfEnd = GetEndWT()->WhichSide(*intSeg->GetStartWT(), 
                                                   *intSeg->GetEndWT());
    
    return sideOfEnd > eps;
}

void IntersectionSegment::SetWCoords() {

    SetStartWT(GetPFace()->TransformToWT(*GetStartXYT()));
    SetEndWT(GetPFace()->TransformToWT(*GetEndXYT()));
}

Point3D IntersectionSegment::Evaluate(const double t) const {

    // We compute the intersection point
    // of the horizontal plane, defined by t, and this segment.
    
    // Precondition:
    // t is between t_start and t_end.
    assert(NumericUtil::Between(GetStartT(), t, GetEndT()));

    // Point3D pointInPlane(0.0, 0.0, t);
    // Vector3D normalVectorOfPlane(0.0, 0.0, 1.0);
    // Vector3D u = *this->GetEndXYT() - *this->GetStartXYT();
    // Vector3D w = this->GetStartXYT() - pointInPlane;
    // double d = normalVectorOfPlane * u;
    // double n = -normalVectorOfPlane * w;
    
    // This can be simplified to:

    const Vector3D u = *GetEndXYT() - *GetStartXYT();
    const double d = GetEndT() - GetStartT();
    const double n = t - GetStartT();

    // This segment must not be parallel to plane:
    assert(!NumericUtil::NearlyEqual(d, 0.0));

    const double s = n / d;

    // This segment intersects the plane:
    assert(NumericUtil::Between(0.0, s, 1.0));

    // Compute segment intersection point:
    return *GetStartXYT() + s * u;
}

void IntersectionSegment::Print() const {
    
    cout << "ID: " << GetID() << " ";
    cout << *GetStartXYT() << " -> " << *GetEndXYT();
    //cout << *GetStartWT() << " -> " << *GetEndWT();
}

string IntersectionSegment::GetVRMLDesc() {
    
    return GetStartXYT()->GetVRMLDesc() + GetEndXYT()->GetVRMLDesc();
}

/*
1 Class MSegment

*/

MSegment::MSegment(const Segment3D& _initial,
                   const Segment3D& _median,
                   const Segment3D& _final,
                   const PFace* const _pFace) :
                 
                 initial(_initial),
                 median(_median),
                 final(_final),
                 pFace(_pFace) {
        
    assert(pFace->IsCoplanarTo(_initial));
    assert(pFace->IsCoplanarTo(_median));
    assert(pFace->IsCoplanarTo(_final));

    assert(!(initial.GetStart() == initial.GetEnd() && final.GetStart()
            == final.GetEnd()));
    
    //assert(initial.IsParallel(final));

    // Create and init the median halfsegment:
    Point m1(true, _median.GetStart().GetX(), _median.GetStart().GetY());
    Point m2(true, _median.GetEnd().GetX(), _median.GetEnd().GetY());

    medianHS = HalfSegment(true, m1, m2);
    insideAbove = medianHS.attr.insideAbove = m1 > m2;
    medianHS.attr.faceno = -1;
    medianHS.attr.cycleno = -1;
    medianHS.attr.edgeno = -1;
    medianHS.attr.coverageno = -1;
    medianHS.attr.partnerno = -1;
}

bool MSegment::IsParallel(const MSegment& ms) const {

    return GetPFace()->IsParallelTo(*ms.GetPFace());
}

void MSegment::Print() const {
        
        cout << "start: " << NList(GetStartAsListExpr()) << endl;
        cout << "end: " << NList(GetEndAsListExpr()) << endl;
        cout << "median: " << this->GetMedianHS() << endl;
        cout << " F: " << GetFaceNo();
        cout << " C: " << GetCycleNo();
        cout << " S: " << GetSegmentNo();
        cout << " LDP: " << GetMedianHS().IsLeftDomPoint() << endl;
    }

/*
1 Class MSegmentCritical

*/

bool MSegmentCritical::IsPartOfUnitA() const {

    return GetPFace()->IsPartOfUnitA();
}

bool MSegmentCritical::HasEqualNormalVector(const MSegmentCritical& msc) const {

    return GetPFace()->HasEqualNormalVector(*msc.GetPFace());
}

/*
1 Class ResultUnit

*/

void ResultUnit::EndBulkLoad(bool merge) {

#ifdef PRINT_STATISTIC
        
        StopWatch stopWatch;
        stopWatch.start();
#endif

    if (IsEmpty())
        return;

    // First, we sort the msegments of this unit by their 
    // median-halfsegments. Comparison between halfsegments
    // is done by the < operator, implemented in the SpatialAlgebra.
    sort(msegments.begin(), msegments.end(), ResultUnit::Less);

    // Second, we construct a region from all median-halfsegments
    // of each msegment of this unit:
    Region r(msegments.size());

    r.StartBulkLoad();

    for (unsigned int i = 0; i < msegments.size(); i++) {

        //cout << msegments[i].GetMedianHS() << endl;
        r.Put(i, msegments[i].GetMedianHS());
    }

    // Note: Sorting is already done.
    r.EndBulkLoad(false, true, true, true);

    // Third, we retrive the faceNo, cycleNo and edgeNo of
    // each halfsegment from the region, computed in the 
    // Region::EndBulkLoad procedure:
    for (unsigned int i = 0; i < msegments.size(); i++) {

        HalfSegment hs;
        r.Get(i, hs);
        msegments[i].CopyIndicesFrom(&hs);
    }

    // Sort msegments by faceno, cycleno and segmentno:
    sort(msegments.begin(), msegments.end(), ResultUnit::LogicLess);
    //this->Print();

    // Erase the second half of msegments, 
    // which contains all MSegments with right dominating point:
    msegments.erase(msegments.begin() + msegments.size() / 2, msegments.end());

    if (merge) {

        // Not implemented yet.
    }

    //this->Print();
    
#ifdef PRINT_STATISTIC
    statistic.durationEndBulkloadOfResultUnit += stopWatch.diffSecondsReal();
#endif
    
}

const ListExpr ResultUnit::ConvertToListExpr() const {

    const int num = msegments.size();

    ListExpr faces = nl->TheEmptyList();
    ListExpr facesLastElem = faces;

    ListExpr face = nl->TheEmptyList();
    ListExpr faceLastElem = face;
    ListExpr cycle = nl->TheEmptyList();
    ListExpr cycleLastElem = cycle;

    for (int i = 0; i < num; i++) {

        const MSegment* dms = &msegments[i];
        const MSegment* nextDms;

        ListExpr p = dms->GetStartAsListExpr();

        if (cycle == nl->TheEmptyList()) {

            cycle = nl->OneElemList(p);
            cycleLastElem = cycle;

        } else {

            cycleLastElem = nl->Append(cycleLastElem, p);
        }

        if (i < num - 1) {

            nextDms = &msegments[i + 1];
        }

        if (i == num - 1 || dms->GetCycleNo() != nextDms->GetCycleNo()
                || dms->GetFaceNo() != nextDms->GetFaceNo()) {

            if (face == nl->TheEmptyList()) {

                face = nl->OneElemList(cycle);
                faceLastElem = face;

            } else {

                faceLastElem = nl->Append(faceLastElem, cycle);
            }

            if (i == num - 1 || dms->GetFaceNo() != nextDms->GetFaceNo()) {

                if (faces == nl->TheEmptyList()) {
                    faces = nl->OneElemList(face);
                    facesLastElem = faces;

                } else
                    facesLastElem = nl->Append(facesLastElem, face);

                face = nl->TheEmptyList();
                faceLastElem = face;
            }

            cycle = nl->TheEmptyList();
            cycleLastElem = cycle;
        }
    }

    ListExpr res = nl->TwoElemList(nl->FourElemList(OutDateTime(
            nl->TheEmptyList(), SetWord((void*) &interval.start)), OutDateTime(
            nl->TheEmptyList(), SetWord((void*) &interval.end)),
            nl->BoolAtom(interval.lc), nl->BoolAtom(interval.rc)), faces);

    return res;
}
    
    URegionEmb* 
    ResultUnit::ConvertToURegionEmb(DbArray<MSegmentData>* segments) const {
        
#ifdef PRINT_STATISTIC
        
        StopWatch stopWatch;
        stopWatch.start();
#endif

        const unsigned int segmentsStartPos = segments->Size();

        URegionEmb* uregion = new URegionEmb(interval, segmentsStartPos);

        double minX = MAX_DOUBLE;
        double maxX = MIN_DOUBLE;
        double minY = MAX_DOUBLE;
        double maxY = MIN_DOUBLE;

        for (unsigned int i = 0; i < msegments.size(); i++) {
            
            const MSegment* mSeg = &msegments[i];

            MSegmentData msd(mSeg->GetFaceNo(), 
                             mSeg->GetCycleNo(), 
                             mSeg->GetSegmentNo(), 
                             mSeg->GetInsideAbove(), 
                             mSeg->GetInitial().GetStart().GetX(), 
                             mSeg->GetInitial().GetStart().GetY(), 
                             mSeg->GetInitial().GetEnd().GetX(), 
                             mSeg->GetInitial().GetEnd().GetY(), 
                             mSeg->GetFinal().GetStart().GetX(), 
                             mSeg->GetFinal().GetStart().GetY(), 
                             mSeg->GetFinal().GetEnd().GetX(), 
                             mSeg->GetFinal().GetEnd().GetY());
            
            //msd.SetDegeneratedInitial(DGM_NONE);
            //msd.SetDegeneratedFinal(DGM_NONE);
            //uregion->PutSegment(segments, i, msd, true);
            
            AddMSegmentData(uregion, segments, msd);
            

            // Update the min/max values:
            minX = min(minX, msd.GetInitialStartX());
            minX = min(minX, msd.GetInitialEndX());
            minX = min(minX, msd.GetFinalStartX());
            minX = min(minX, msd.GetFinalEndX());

            minY = min(minY, msd.GetInitialStartY());
            minY = min(minY, msd.GetInitialEndY());
            minY = min(minY, msd.GetFinalStartY());
            minY = min(minY, msd.GetFinalEndY());

            maxX = max(maxX, msd.GetInitialStartX());
            maxX = max(maxX, msd.GetInitialEndX());
            maxX = max(maxX, msd.GetFinalStartX());
            maxX = max(maxX, msd.GetFinalEndX());

            maxY = max(maxY, msd.GetInitialStartY());
            maxY = max(maxY, msd.GetInitialEndY());
            maxY = max(maxY, msd.GetFinalStartY());
            maxY = max(maxY, msd.GetFinalEndY());

        }
        

        // Set the bbox:
        
        double min[3] = { minX, minY, interval.start.ToDouble() };
        double max[3] = { maxX, maxY, interval.end.ToDouble() };

        uregion->SetBBox(Rectangle<3>(true, min, max));
        
/*
We have to go through the lists of degenerated segments and count 
how often
a region is inside above in each list. If there are more inside above
segments than others, we need one inside above segment for the
~TemporalFunction()~ and set all others to ignore. Vice version if 
there are more inside below segments. If there is the same number of 
inside above and inside below segments, we can ignore the entire list.

*/
        
        //bool nonTrivialInitial = false;
        //bool nonTrivialFinal = false;
        
        for (int i = 0; i < uregion->GetSegmentsNum(); i++) {

            MSegmentData auxDms;
            uregion->GetSegment(segments, i, auxDms);
            MSegmentData dms( auxDms);

            //if (!dms.GetPointInitial() && dms.GetDegeneratedInitialNext() < 0)
            //    nonTrivialInitial = true;

            //if (!dms.GetPointFinal() && dms.GetDegeneratedFinalNext() < 0)
            //    nonTrivialFinal = true;

            if (dms.GetDegeneratedInitial() == DGM_UNKNOWN) {

                if (dms.GetDegeneratedInitialNext() >= 0) {

                    MSegmentData auxDegenDms;
                    MSegmentData degenDms;
                    unsigned int numInsideAbove = 0;
                    unsigned int numNotInsideAbove = 0;

                    for (int j = i+1; j != 0; j
                            = degenDms.GetDegeneratedInitialNext()) {

                        uregion->GetSegment(segments, j-1, auxDegenDms);
                        degenDms = auxDegenDms;

                        if (degenDms.GetInsideAbove())
                            numInsideAbove++;
                        else
                            numNotInsideAbove++;

                        if (j != i+1) {

                            degenDms.SetDegeneratedInitial(DGM_IGNORE);
                            uregion->PutSegment(segments, j-1, degenDms);
                        }
                    }

                    if (numInsideAbove == numNotInsideAbove) {

                        dms.SetDegeneratedInitial(DGM_IGNORE);

                    } else if (numInsideAbove == numNotInsideAbove+1) {

                        dms.SetDegeneratedInitial(DGM_INSIDEABOVE);

                    } else if (numInsideAbove+1 == numNotInsideAbove) {

                        dms.SetDegeneratedInitial(DGM_NOTINSIDEABOVE);

                    } else {

                        cerr << "segment (" << dms.GetInitialStartX() << ", "
                                << dms.GetInitialStartY() << ")-("
                                << dms.GetInitialEndX() << ", "
                                << dms.GetInitialEndY() << ") / ("
                                << dms.GetFinalStartX() << ", "
                                << dms.GetFinalStartY() << ")-("
                                << dms.GetFinalEndX() << ", "
                                << dms.GetFinalEndY()
                                << ") incorrectly degenerated "
                                << "in initial instant" << endl;

                        delete uregion;
                        return 0;
                    }

                } else
                    dms.SetDegeneratedInitial(DGM_NONE);
            }

            if (dms.GetDegeneratedFinal() == DGM_UNKNOWN) {

                if (dms.GetDegeneratedFinalNext() >= 0) {

                    MSegmentData auxDegenDms;
                    MSegmentData degenDms;
                    unsigned int numInsideAbove = 0;
                    unsigned int numNotInsideAbove = 0;

                    for (int j = i+1; j != 0; j
                            = degenDms.GetDegeneratedFinalNext()) {

                        //cout << segments->Size()-1 << " " << j-1 << endl;
                        uregion->GetSegment(segments, j-1, auxDegenDms);
                        degenDms = auxDegenDms;

                        if (degenDms.GetInsideAbove())
                            numInsideAbove++;
                        else
                            numNotInsideAbove++;

                        if (j != i+1) {

                            degenDms.SetDegeneratedFinal(DGM_IGNORE);
                            uregion->PutSegment(segments, j-1, degenDms);
                        }
                    }

                    if (numInsideAbove == numNotInsideAbove) {

                        dms.SetDegeneratedFinal(DGM_IGNORE);

                    } else if (numInsideAbove == numNotInsideAbove+1) {

                        dms.SetDegeneratedFinal(DGM_INSIDEABOVE);

                    } else if (numInsideAbove+1 == numNotInsideAbove) {

                        dms.SetDegeneratedFinal(DGM_NOTINSIDEABOVE);

                    } else {

                        cerr << "segment (" << dms.GetInitialStartX() << ", "
                                << dms.GetInitialStartY() << ")-("
                                << dms.GetInitialEndX() << ", "
                                << dms.GetInitialEndY() << ") / ("
                                << dms.GetFinalStartX() << ", "
                                << dms.GetFinalStartY() << ")-("
                                << dms.GetFinalEndX() << ", "
                                << dms.GetFinalEndY()
                                << ") incorrectly degenerated "
                                << "in final instant" << endl;

                        delete uregion;
                        return 0;
                    }

                } else
                    dms.SetDegeneratedFinal(DGM_NONE);
            }

            uregion->PutSegment(segments, i, dms);
        }

#ifdef PRINT_STATISTIC
    statistic.durationConvertResultUnitToURegionEmb
            += stopWatch.diffSecondsReal();
#endif
        
        //assert(Check());
        return uregion;
    }
    
void ResultUnit::Print(const bool segments) const {

    cout << "unit: " << interval << endl << endl;

    if (segments)
        for (unsigned int i = 0; i < msegments.size(); i++) {

            msegments[i].Print();
        }
}

string ResultUnit::GetVRMLDesc() const {

        std::ostringstream oss;
        
        oss << endl;
        oss << "# Interval: " << interval << endl;
        oss << "# Interval: " << interval.start.ToDouble();
        oss << " -> " << interval.end.ToDouble() << endl;
        oss << endl;
        
        oss << endl;
        oss << "# Unit :" << endl;
        oss << endl;
        
        const double scale = VRML_SCALE_FACTOR;

        oss << "Transform {" << endl;
        oss << "\tscale " << scale << " " << scale << " " << scale << endl;
        oss << "\tchildren [" << endl;
        oss << "\t\tShape {" << endl;
        oss << "\t\t\tgeometry IndexedLineSet {" << endl;
        oss << "\t\t\t\tcoord Coordinate {" << endl;
        oss << "\t\t\t\t\tpoint [" << endl;

        int count = 0;
        vector<MSegment>::const_iterator iter;
        
        for (iter = msegments.begin(); iter != msegments.end(); iter++) {

            const MSegment ms = *iter;
            
            const Point3D a(ms.GetInitial().GetStart().GetX(),
                            ms.GetInitial().GetStart().GetY(),
                            interval.start.ToDouble());
            const Point3D b(ms.GetInitial().GetEnd().GetX(),
                            ms.GetInitial().GetEnd().GetY(),
                            interval.start.ToDouble());
            const Point3D c(ms.GetFinal().GetStart().GetX(),
                            ms.GetFinal().GetStart().GetY(),
                            interval.end.ToDouble());
            const Point3D d(ms.GetFinal().GetEnd().GetX(),
                            ms.GetFinal().GetEnd().GetY(),
                            interval.end.ToDouble());
            
            oss << "\t\t\t\t\t\t" << a.GetVRMLDesc() 
                                     << c.GetVRMLDesc()
                                     << d.GetVRMLDesc()
                                     << b.GetVRMLDesc()
                                     << endl;
            count++;
        }

        oss << "\t\t\t\t\t]  # end point" << endl;
        oss << "\t\t\t\t} # end coord" << endl;
        oss << "\t\t\t\tcoordIndex [" << endl;

        const int noPoints = count * 4;

        for (int i = 0; i < noPoints; i += 4) {

            oss << "\t\t\t\t\t" << i << ", " << i + 1 << ", " << i + 2
                    << ", " << i + 3 << ", " << i << ", " << "-1," << endl;
        }

        oss << "\t\t\t\t] # end coordIndex" << endl;

        //oss << "\t\t\t\tcolor Color { color [ " << color << " ] }" << endl;

        oss << "\t\t\t} # end geometry" << endl;
        oss << "\t\t} # end shape" << endl;
        oss << "\t] # end children" << endl;
        oss << "} # end Transform" << endl;
        
        return oss.str();
}

void ResultUnit::AddMSegmentData(URegionEmb* uregion,
                                 DbArray<MSegmentData>* segments, 
                                 MSegmentData& dms) const {
        
/*
For each of the already existing segments:

*/
        
        const int segmentsNum = uregion->GetSegmentsNum();
        const int segmentsStartPos = uregion->GetStartPos();
        
                for (int i = segmentsNum - 1; i >= 0; i--) {

                    MSegmentData auxExistingDms;

                    segments->Get(segmentsStartPos + i, auxExistingDms);
                    MSegmentData existingDms( auxExistingDms );

/*
Check whether the current segment degenerates with this segment in the
initial instant. Note that segments reduced to points are excluded from
this.

All segments, which degenerate into each other, are collected in a list
using the ~degeneratedInitialNext~ attribute.

*/
                    if (dms.GetDegeneratedInitialNext() < 0
                        && !dms.GetPointInitial()
                        && !existingDms.GetPointInitial()
                        && ((NumericUtil::NearlyEqual(
                                 dms.GetInitialStartX(),
                                 existingDms.GetInitialStartX())
                             && NumericUtil::NearlyEqual(
                                    dms.GetInitialStartY(),
                                    existingDms.GetInitialStartY())
                             && NumericUtil::NearlyEqual(
                                    dms.GetInitialEndX(),
                                    existingDms.GetInitialEndX())
                             && NumericUtil::NearlyEqual(
                                    dms.GetInitialEndY(),
                                    existingDms.GetInitialEndY()))
                            || (NumericUtil::NearlyEqual(
                                    dms.GetInitialStartX(),
                                    existingDms.GetInitialEndX())
                                && NumericUtil::NearlyEqual(
                                        dms.GetInitialStartY(),
                                        existingDms.GetInitialEndY())
                                && NumericUtil::NearlyEqual(
                                        dms.GetInitialEndX(),
                                        existingDms.GetInitialStartX())
                                && NumericUtil::NearlyEqual(
                                        dms.GetInitialEndY(),
                                        existingDms.GetInitialStartY())))) {

                        dms.SetDegeneratedInitialNext(0);
                        existingDms.SetDegeneratedInitialNext(segmentsNum + 1);

                        segments->Put(segmentsStartPos + i, existingDms);
                    }

/*
Same for the final instant.

*/
                    if (dms.GetDegeneratedFinalNext() < 0
                        && !dms.GetPointFinal()
                        && !existingDms.GetPointFinal()
                        && ((NumericUtil::NearlyEqual(
                                 dms.GetFinalStartX(),
                                 existingDms.GetFinalStartX())
                             && NumericUtil::NearlyEqual(
                                    dms.GetFinalStartY(),
                                    existingDms.GetFinalStartY())
                             && NumericUtil::NearlyEqual(
                                    dms.GetFinalEndX(),
                                    existingDms.GetFinalEndX())
                             && NumericUtil::NearlyEqual(
                                    dms.GetFinalEndY(),
                                    existingDms.GetFinalEndY()))
                            || (NumericUtil::NearlyEqual(
                                    dms.GetFinalStartX(),
                                    existingDms.GetFinalEndX())
                                && NumericUtil::NearlyEqual(
                                       dms.GetFinalStartY(),
                                       existingDms.GetFinalEndY())
                                && NumericUtil::NearlyEqual(
                                       dms.GetFinalEndX(),
                                       existingDms.GetFinalStartX())
                                && NumericUtil::NearlyEqual(
                                       dms.GetFinalEndY(),
                                       existingDms.GetFinalStartY())))) {
                        
                        dms.SetDegeneratedFinalNext(0);
                        existingDms.SetDegeneratedFinalNext(segmentsNum + 1);

                        segments->Put(segmentsStartPos + i, existingDms);
                    }
                }
        
        segments->resize(segmentsStartPos + segmentsNum + 1);
        segments->Put(segmentsStartPos + segmentsNum, dms);
        uregion->SetSegmentsNum(segmentsNum + 1);
    }

bool ResultUnit::Check() const {

    for (unsigned int i = 0; i < msegments.size() - 1; i++) {

        const MSegment* ms1 = &msegments[i];
        const MSegment* ms2 = &msegments[i+1];

        const Point2D a1(ms1->GetInitial().GetStart());
        const Point2D c1(ms1->GetFinal().GetStart());

        const Point2D a2(ms2->GetInitial().GetStart());
        const Point2D c2(ms2->GetFinal().GetStart());

        if (a1 == a2 && c1 == c2) {

            flippedPFaces.insert(msegments[i-1].GetPFace()->GetID());
            flippedPFaces.insert(ms1->GetPFace()->GetID());
        }
    }

    return flippedPFaces.empty();
}

/*
1 Class Point3DExt

*/

bool Point3DExt::operator <(const Point3DExt& p) const {

    if (NumericUtil::Lower(GetX(), p.GetX()))
        return true;

    if (NumericUtil::Greater(GetX(), p.GetX()))
        return false;

    if (NumericUtil::Lower(GetY(), p.GetY()))
        return true;

    if (NumericUtil::Greater(GetY(), p.GetY()))
        return false;

    if (NumericUtil::Lower(GetT(), p.GetT()))
        return true;

    if (NumericUtil::Greater(GetT(), p.GetT()))
        return false;

    //cout << "sourceFlag < p.sourceFlag" << endl;
    return sourceFlag < p.sourceFlag;
}

bool PointExtSet::GetIntersectionSegment(Segment3D& result) const {

    if (s.size() != 4)
        return false;

    set<Point3DExt>::iterator it = s.begin();

    Point3DExt p1 = *it;
    it++;
    Point3DExt p2 = *it;

    if (p1.sourceFlag == p2.sourceFlag)
        return false;

    it++;
    Point3DExt p3 = *it;

    if (p2 == p3) {

        // The length of the intersection segment is zero.
        return false;
    }

    result = Segment3D(p2, p3);
    return true;
}

void PointExtSet::Print() const {
    
    set<Point3DExt>::iterator iter;
    
    for (iter = s.begin(); iter != s.end(); ++iter) {

        cout << *iter << endl;
    }
}

/*
1 Struct IntSegCompare

*/

bool IntSegCompare::operator()(const IntersectionSegment* const& s1,
        const IntersectionSegment* const& s2) const {

    // We sort by (t_start, w_start, IsLeft())

    // Precondition: s1->GetStartT() < s1->GetEndT() && 
    //               s2->GetStartT() < s2->GetEndT()

    if (NumericUtil::Lower(s1->GetStartT(), s2->GetStartT()))
        return true;

    if (NumericUtil::Greater(s1->GetStartT(), s2->GetStartT()))
        return false;
    
    // s1->GetStartT() == s2->GetStartT()
    
    if (NumericUtil::Lower(s1->GetStartW(), s2->GetStartW()))
        return true;

    if (NumericUtil::Greater(s1->GetStartW(), s2->GetStartW()))
        return false;

    // s1->GetStartW() == s2->GetStartW()
    
    if (*(s1->GetEndWT()) == *(s2->GetEndWT()))
        return true;
    
    //return s1->IsLeftOf(s2);
    return s1->GetEndWT()->IsLeft(*(s2->GetStartWT()), *(s2->GetEndWT()));
    
}

/*
1 Class IntSegContainer

*/

IntSegContainer::~IntSegContainer() {

    set<IntersectionSegment*>::iterator iter;

    for (iter = intSegs.begin(); iter != intSegs.end(); ++iter) {

        delete *iter;
    }
}

void IntSegContainer::UpdateTimeLevel(double _t) {
    
    if (firstTimeLevel) {
        
        intSegIter = intSegs.begin();
        firstTimeLevel = false;
    }
    
    t = _t;
    activeIter = active.begin();

    while (activeIter != active.end()) {

        while (activeIter != active.end() && IsOutOfRange(*activeIter)) {

            activeIter = active.erase(activeIter);
        }

        if (activeIter == active.end())
            break;

        if (HasMoreSegsToInsert()) {

            IntersectionSegment* newSeg = *intSegIter;

            if (newSeg->IsLeftOf(*activeIter)) {

                activeIter = active.insert(activeIter, newSeg);
                intSegIter++;
            }
        }

        activeIter++;
    }

    assert(activeIter == active.end());
    
    // Add the tail, if there is one:
    while (HasMoreSegsToInsert()) {
        
        IntersectionSegment* newSeg = *intSegIter;
        activeIter = active.insert(activeIter, newSeg);
        intSegIter++;
        activeIter = active.end();
    }
}

void IntSegContainer::Print() const {
    
    if (intSegs.empty()) {
        
        cout << "Empty." << endl;
        return;
    }

    set<IntersectionSegment*>::const_iterator iter;

    for (iter = intSegs.begin(); iter != intSegs.end(); ++iter) {

        //cout << (*iter)->GetID() << " ";
        (*iter)->Print();
        cout << endl;
    }
}

void IntSegContainer::PrintActive() const {

    list<IntersectionSegment*>::const_iterator iter;

    for (iter = active.begin(); iter != active.end(); ++iter) {

        cout << (*iter)->GetID() << " ";
        //(*iter)->Print();
    }
}

/*
1 Class PFace

*/

unsigned int PFace::instanceCount = 0;

PFace::PFace(SourceUnit* _unit, const MSegmentData* _mSeg) :

    unit(_unit), 
    mSeg(_mSeg),
    //insideAbove(_mSeg->GetInsideAbove()),
    hasInnerIntSegs(false),
    state(UNKNOWN),
    id(instanceCount++) {

    SetInitialStartPointIsA();
    ComputeBoundingRect();
    ComputeNormalVector();
    ComputeWTCoords();
}

void PFace::SetInitialStartPointIsA() {
    
    Point2D start;
    Point2D end;
    
    if (!mSeg->GetPointInitial()) {
        
        start = Point2D(mSeg->GetInitialStartX(), mSeg->GetInitialStartY());
        end = Point2D(mSeg->GetInitialEndX(), mSeg->GetInitialEndY());
        
    } else {
        
        start = Point2D(mSeg->GetFinalStartX(), mSeg->GetFinalStartY());
        end = Point2D(mSeg->GetFinalEndX(), mSeg->GetFinalEndY());
    }
    
    initialStartPointIsA = (start < end) == mSeg->GetInsideAbove();
}

bool PFace::IntersectionPlaneSegment(const Segment3D& seg, Point3D& result) {

    // Precondition: 

    // We compute the intersection point
    // of the plane - defined by the PFace - and the segment.

    Vector3D u = seg.GetEnd() - seg.GetStart();
    Vector3D w = seg.GetStart() - this->GetA_XYT();

    double d = this->GetNormalVector() * u;
    double n = -this->GetNormalVector() * w;

    if (NumericUtil::NearlyEqual(d, 0.0)) // Segment is parallel to plane.
        return false;

    double s = n / d;

    // No intersection point, if s < -eps or s > 1 + eps.
    if (NumericUtil::Lower(s, 0.0) || NumericUtil::Greater(s, 1.0)) 
    //const double eps = 0.0001;
    //if (s < -eps || s > 1 + eps)
        return false;

    // Compute segment intersection point:
    result = seg.GetStart() + s * u;

    return true;
}

void PFace::Intersection(PFace& other) {
    
    assert(IsPartOfUnitA());
    
#ifdef OPTIMIZE_BY_BOUNDING_RECT
    
    if (!GetBoundingRect().Intersects(other.GetBoundingRect()))
        return;
    
#endif
    
    if (this->IsParallelTo(other)) {

        if (this->IsCoplanarTo(other)) {

            this->MarkAsCriticalRelevant();
            other.MarkAsCriticalRelevant();

            //cout << "Coplanar PFace pair found." << endl;
            
        } else {
        
            //cout << "Parallel PFace pair found." << endl;
        }

        return;
    }

    // We store all edges of this PFace as 3DSegments in the vector edgesPFaceA:
    vector<Segment3D> edgesPFaceA;

    edgesPFaceA.push_back(Segment3D(this->GetA_XYT(), this->GetC_XYT()));
    edgesPFaceA.push_back(Segment3D(this->GetB_XYT(), this->GetD_XYT()));

    if (!this->GetMSeg()->GetPointInitial())
        edgesPFaceA.push_back(Segment3D(this->GetA_XYT(), this->GetB_XYT()));

    if (!this->GetMSeg()->GetPointFinal())
        edgesPFaceA.push_back(Segment3D(this->GetC_XYT(), this->GetD_XYT()));

    // We store all edges of the other PFace as 3DSegments
    // in the vector edgesPFaceB:
    vector<Segment3D> edgesPFaceB;

    edgesPFaceB.push_back(Segment3D(other.GetA_XYT(), other.GetC_XYT()));
    edgesPFaceB.push_back(Segment3D(other.GetB_XYT(), other.GetD_XYT()));

    if (!other.GetMSeg()->GetPointInitial())
        edgesPFaceB.push_back(Segment3D(other.GetA_XYT(), other.GetB_XYT()));

    if (!other.GetMSeg()->GetPointFinal())
        edgesPFaceB.push_back(Segment3D(other.GetC_XYT(), other.GetD_XYT()));

    PointExtSet intPointSet;
    Point3DExt intPoint;

    // Intersect the plane - defined by the other PFace - 
    // with all edges of this PFace:
    unsigned int i = 0;
    while (intPointSet.Size() < 2 && i < edgesPFaceA.size()) {

        if (other.IntersectionPlaneSegment(edgesPFaceA[i], intPoint)) {

            intPoint.sourceFlag = PFACE_A;
            intPointSet.Insert(intPoint);
        }

        i++;
    }

    if (intPointSet.Size() != 2) // We need exactly two intersection points.
        return;

    // Intersect the plane - defined by this PFace - 
    // with all edges of the other PFace:
    unsigned int j = 0;
    while (intPointSet.Size() < 4 && j < edgesPFaceB.size()) {

        if (this->IntersectionPlaneSegment(edgesPFaceB[j], intPoint)) {

            intPoint.sourceFlag = PFACE_B;
            intPointSet.Insert(intPoint);
        }

        j++;
    }

    Segment3D intSeg;
    
    if (!intPointSet.GetIntersectionSegment(intSeg))
        return; // There is no intersection.
    
#ifdef PRINT_DEBUG_MESSAGES
    
    if (intSeg.Length() < 0.1) {
        
        cout << "intSeg.Length(): " << intSeg.Length() << endl;
        cout << "intSeg: " << intSeg << endl;
        cout << "intPointSet: " << endl;
        intPointSet.Print();
    }
    
#endif
    
    // We add one intersection segment to each PFace:
    this->AddIntSeg(intSeg, other);
    other.AddIntSeg(intSeg, *this);
    
    //statistic.noIntSegsTotal += 2;
}

void PFace::ComputeBoundingRect() {

    pair<double, double> x = NumericUtil::MinMax4(mSeg->GetInitialStartX(),
                                                  mSeg->GetInitialEndX(), 
                                                  mSeg->GetFinalStartX(),
                                                  mSeg->GetFinalEndX());

    pair<double, double> y = NumericUtil::MinMax4(mSeg->GetInitialStartY(),
                                                  mSeg->GetInitialEndY(), 
                                                  mSeg->GetFinalStartY(),
                                                  mSeg->GetFinalEndY());

    double minMax[] = {x.first - NumericUtil::eps, 
                       x.second + NumericUtil::eps, 
                       y.first - NumericUtil::eps, 
                       y.second + NumericUtil::eps};
    boundingRect = Rectangle<2>(true,minMax );
}

void PFace::ComputeWTCoords() {

    // The vector w is either the normalized cross product 
    // of the normal vector and the t-unit-vector, or it's opposite.
    // This depends on the kind of set-operation, we want to perform.

    // wVector = Vector3D(GetNormalVector() ^ Vector3D(0.0, 0.0, 1.0);
    // wVector.Normalize();

    // This can be simplified to:
    wVector = Vector3D(GetNormalVector().GetY(), 
                       - GetNormalVector().GetX(),
                       0.0);
    
    // Usually, we look at the PFace from the inside.
    
    if (GetOperation() == MINUS && IsPartOfUnitB()) {
        
        // Only in this case, we want to look from the outside.
        wVector = - wVector;
    }
    
    wVector.Normalize();

    // For the sake of efficency, we cache the w-coords of some points:
    aW = TransformToW(this->GetA_XYT());
    bW = TransformToW(this->GetB_XYT());
    cW = TransformToW(this->GetC_XYT());
    dW = TransformToW(this->GetD_XYT());
}

void PFace::ComputeNormalVector() {

    if (!AEqualsB()) {

        // Cross product of vector AB and AC:
        normalVector = (GetB_XYT() - GetA_XYT()) ^ (GetC_XYT() - GetA_XYT());

    } else { // A == B

        // Cross product of vector DC and DB:
        normalVector = (GetC_XYT() - GetD_XYT()) ^ (GetB_XYT() - GetD_XYT());
    }

    normalVector.Normalize();
}

void PFace::DecideRelevanceAndAddBoundary() {
    
    if (!DECIDE_BY_ENTIRELY_IN_OUT) {
        
        if (GetState() == UNKNOWN || IsIrrelevant())
            MarkAsNormalRelevant();
    }
    
    if (IsIrrelevant()) {

        return;
    }

    if (IsNormalRelevant() || IsCriticalRelevant()) {

        AddEntireBoundary();
        return;
    }

    // The PFace has no intersection segments and it's unknown, if
    // it is relevant for the result.

    // Check, if the PFace is outside or inside 
    // of the other region unit
    // and add intSegs depending of the operation.
    
    const SetOp op = unit->GetOperation();

    if (unit->GetPartner()->IsEntirelyOutside(this)) {

        //MarkAsEntirelyOutside();

        if (op == UNION || (op == MINUS && IsPartOfUnitA())) {

            AddEntireBoundary();

        } else {

            // op == INTERSECTION || (op == MINUS && IsPartOfUnitB()):
            MarkAsIrrelevant();
        }

    } else { // The PFace is entirely inside of the other unit.

        //MarkAsEntirelyInside();

        if (op == INTERSECTION || (op == MINUS && IsPartOfUnitB())) {

            AddEntireBoundary();

        } else {

            // op == UNION || (op == MINUS && IsPartOfUnitA()):
            MarkAsIrrelevant();
        }
    }
}

void PFace::AddEntireBoundary() {
    
    AddBoundaryIntSeg(Segment3D(GetA_XYT(), GetC_XYT()));
    AddBoundaryIntSeg(Segment3D(GetB_XYT(), GetD_XYT()));
}

bool PFace::IsParallelTo(const PFace& pf) const {

    Vector3D cross = GetNormalVector() ^ pf.GetNormalVector();
    //cross.Normalize();
    
    return cross.IsZero();
}

bool PFace::IsCoplanarTo(const PFace& pf) const {
    
    // Precondition: this is parallel to pf.
    
    return IsCoplanarTo(pf.GetA_XYT());
}

bool PFace::IsCoplanarTo(const Point3D& p) const {
    
    const double distance2 = p.DistanceToPlane2(GetA_XYT(), GetNormalVector());
    
    return NumericUtil::NearlyEqual(distance2, 0.0);
}

bool PFace::IsCoplanarTo(const Segment3D& seg) const {
    
    return IsCoplanarTo(seg.GetStart()) && IsCoplanarTo(seg.GetEnd());
}

bool PFace::IsCoplanarTo(const IntersectionSegment& seg) const {

    return IsCoplanarTo(Segment3D(*seg.GetStartXYT(), *seg.GetEndXYT()));
}

bool PFace::HasEqualNormalVector(const PFace& pf) const {
    
    // Precondition: this is parallel to pf.
    
    // The normal vectors are equal, iff
    // the cosinus of the angle between them is positive:
    return GetNormalVector() * pf.GetNormalVector() > 0.0;
}

bool PFace::IsColinearToBoundary(const Segment3D& s) const {
    
    return GetRelationToBoundary(s, LEFT_BOUNDARY) == COLINEAR ||
           GetRelationToBoundary(s, RIGHT_BOUNDARY) == COLINEAR;
}

const RelationToBoundary 
PFace::GetRelationToBoundary(const Segment3D& s, const Boundary b) const {
    
    const Segment2D sWT = TransformToWT(s);
    const Segment2D boundary = GetBoundary(b);
    
    const bool startOnBoundary = sWT.GetStart().IsColinear(boundary);
    const bool endOnBoundary = sWT.GetEnd().IsColinear(boundary);
    
    if (!startOnBoundary && !endOnBoundary) {
        
        return NO_TOUCH;
    }
    
    if (startOnBoundary && endOnBoundary) {

        return COLINEAR;
    }

    if (startOnBoundary) {

        return TOUCH_IN_STARTPOINT;
    }

    // if (endOnBoundary) {

        return TOUCH_IN_ENDPOINT;
    //}
}

const Point3D PFace::GetMidPoint() const {
    
    const Point3D a((GetA_XYT() + GetC_XYT()) * 0.5);
    const Point3D b((GetB_XYT() + GetD_XYT()) * 0.5);
    
    return Point3D((a + b) * 0.5);
}

void PFace::AddIntSeg(const Segment3D& seg, PFace& other) {
    
    const RelationToBoundary rtlb = GetRelationToBoundary(seg, LEFT_BOUNDARY);
    
    switch (rtlb) {
    
    case NO_TOUCH:
        
        MarkAsNormalRelevant();
        
        break;
        
    case TOUCH_IN_STARTPOINT:
        
        MarkAsNormalRelevant();
        intSegs.AddCriticalTime(seg.GetStart().GetT());
        
        break;
        
    case TOUCH_IN_ENDPOINT:
        
        MarkAsNormalRelevant();
        intSegs.AddCriticalTime(seg.GetEnd().GetT());
        
        break;
    
    case COLINEAR:
        
        MarkAsCriticalRelevant();
        other.MarkAsCriticalRelevant();
       
        break;
    }
    
    unit->AddGlobalTimeValue(seg.GetStart().GetT());
    unit->AddGlobalTimeValue(seg.GetEnd().GetT());
    
    hasInnerIntSegs = true;
         
    if (!seg.IsOrthogonalToTAxis()) {
        
        //assert(this->IsCoplanarTo(seg));

        IntersectionSegment* intSeg = new IntersectionSegment(seg);
        
        //assert(this->IsCoplanarTo(*intSeg));
        
        intSeg->SetPFace(this);
        intSeg->SetWCoords();
        //intSeg->SetSideOfResultFace(*this, other);
        
        intSegs.AddIntSeg(intSeg);
    }
}

void PFace::AddBoundaryIntSeg(const Segment3D& seg) {

    MarkAsNormalRelevant();

    unit->AddGlobalTimeValue(seg.GetStart().GetT());
    unit->AddGlobalTimeValue(seg.GetEnd().GetT());

    IntersectionSegment* intSeg = new IntersectionSegment(seg);

    intSeg->SetPFace(this);
    intSeg->SetWCoords();

    intSegs.AddIntSeg(intSeg);
    
    statistic.noBorderIntSegs++;
}

string PFace::GetVRMLDesc() {

    return GetA_XYT().GetVRMLDesc() + 
           GetC_XYT().GetVRMLDesc() + 
           GetD_XYT().GetVRMLDesc() + 
           GetB_XYT().GetVRMLDesc();
}

string PFace::GetStateAsString() const {
    
    const State state = GetState();
    
    switch (state) {
    
    case UNKNOWN:
        return "UNKNOWN";
    
    case ENTIRELY_INSIDE:
            return "ENTIRELY_INSIDE"; 
            
    case ENTIRELY_OUTSIDE:
            return "ENTIRELY_OUTSIDE"; 
        
    case RELEVANT_NOT_CRITICAL:
            return "RELEVANT_NOT_CRITICAL"; 
            
    case RELEVANT_CRITICAL:
            return "RELEVANT_CRITICAL"; 
            
    case NOT_RELEVANT:
            return "NOT_RELEVANT"; 
    }
    
    // Should never be reached:
    return "I don't know this state!";
}

void PFace::Print() const {

    cout << "PFace ID: " << GetID() << endl;
    cout << endl;
    
    //cout << "Starttime: " << unit->GetStartTime() << endl;
    //cout << "Endtime: " << unit->GetEndTime() << endl;
    
    cout << "A_XYT: " << GetA_XYT() << endl;
    cout << "B_XYT: " << GetB_XYT() << endl;
    cout << "C_XYT: " << GetC_XYT() << endl;
    cout << "D_XYT: " << GetD_XYT() << endl;
    cout << endl;
    cout << "A_WT: " << GetA_WT() << endl;
    cout << "B_WT: " << GetB_WT() << endl;
    cout << "C_WT: " << GetC_WT() << endl;
    cout << "D_WT: " << GetD_WT() << endl;
    cout << endl;
    cout << "Normal vector: " << GetNormalVector() << endl;
    cout << "W vector: " << GetWVector() << endl;
    cout << endl;
    cout << "State: " << GetStateAsString() << endl;
    //cout << "mSeg->GetInsideAbove(): " << mSeg->GetInsideAbove() << endl;
    cout << endl;
    cout << "IntersectionSegments: "<< endl;
    intSegs.Print();
    cout << endl;
    
    cout << endl;
    cout << "*********************************************" << endl;
    cout << endl;
}

} // end of namespace mregionops

} // end of namespace temporalalgebra

