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

#include "SetOpUtil.h"

bool Point3DExt::Lower(const Point3DExt& p) const {

    if (NumericUtil::Lower(GetX(), p.GetX()))
        return true;
    else if (NumericUtil::NearlyEqual(GetX(), p.GetX())) {
        if (NumericUtil::Lower(GetY(), p.GetY()))
            return true;
        else if (NumericUtil::NearlyEqual(GetY(), p.GetY())) {
            if (NumericUtil::Lower(GetT(), p.GetT()))
                return true;
            else if (NumericUtil::NearlyEqual(GetT(), p.GetT())) {
                if (sourceFlag < p.sourceFlag)
                    return true;
            }
        }
    }

    return false;
}

pair<IntersectionSegment*, IntersectionSegment*> 
PointExtSet::GetIntersectionSegment() const {

    pair<IntersectionSegment*, IntersectionSegment*>
            noIntersectionSegment(0, 0);

    if (s.size() != 4)
        return noIntersectionSegment;

    set<Point3DExt>::iterator it = s.begin();

    Point3DExt p1 = (Point3DExt)*it;
    it++;
    Point3DExt p2 = (Point3DExt)*it;

    if (p1.sourceFlag == p2.sourceFlag)
        return noIntersectionSegment;

    it++;
    Point3DExt p3 = (Point3DExt)*it;

    if (NumericUtil::NearlyEqual(p2.GetX(), p3.GetX())
            && NumericUtil::NearlyEqual(p2.GetY(), p3.GetY())
            && NumericUtil::NearlyEqual(p2.GetT(), p3.GetT())) {

        // The length of the intersection segment is zero.
        return noIntersectionSegment;
    }

    return IntersectionSegment::createBuddyPair(p2, p3);
}

bool Segment2D::IsAbove(const Point2D& p) const {

    // TODO:
    
}

PUnit::PUnit(const bool _isUnitA, 
             const URegionEmb* const _uRegion,
             const DBArray<MSegmentData>* _array, 
             PUnitPair* const _parent) :
                 
            isUnitA(_isUnitA),
            uRegion(_uRegion), 
            array(_array),
            parent(_parent), 
            startTime(_uRegion->timeInterval.start.ToDouble()),
            endTime(_uRegion->timeInterval.end.ToDouble()),
            medianRegion(0) {

    ComputeBoundingRect();
    hasMedianRegion = false;
    medianTime = GetTimeInterval().start + 
     ((GetTimeInterval().end - GetTimeInterval().start) * 0.5);
    
    cout << "medianTime = " << medianTime.ToDouble() << endl;
}


void PUnit::ComputeBoundingRect() {

    // Calculate the projection bounding rectangle in the (x, y)-plane
    // of the URegion:
    
    double minX = uRegion->BoundingBox().MinD(0);
    double maxX = uRegion->BoundingBox().MaxD(0);
    double minY = uRegion->BoundingBox().MinD(1);
    double maxY = uRegion->BoundingBox().MaxD(1);
    
    boundingRect = Rectangle<2>(true, minX, maxX, minY, maxY);
}

void PUnit::ComputeMedianRegion() {
    
    //Region _medianRegion(0);
    
    GetURegionEmb()->TemporalFunction(array, medianTime, medianRegion);
    
    // medianRegion.Print(cout);
    
    // Reduce the median region, if possible:
    
    // double minX = parent->GetOverlapRect().MinD(0);
    // double maxX = parent->GetOverlapRect().MaxD(0);
    // double minY = uRegion->BoundingBox().MinD(1);
    // double maxY = uRegion->BoundingBox().MaxD(1);
    
    // const Rectangle<2> clippingWindow(true, minX, maxX, minY, maxY);
    
    // medianRegion.WindowClippingIn(clippingWindow, medianRegion);
    
    
    hasMedianRegion = true;
}

void PUnit::AddBoundarySegments(const SetOp op) {

    
    
    bool firstPairInCycle;

    for (list<PFaceCycle>::iterator iter = pFacesCycles.begin(); iter
            != pFacesCycles.end(); iter++) {

        PFaceCycleIterator pairIter = (*iter).GetPFaceCycleIterator();
        firstPairInCycle = true;

        while (pairIter.HasNextPair()) {

            PFacePair pair = pairIter.GetNextPair();
            
            //pair.GetLeft()->Print();
            //pair.GetRight()->Print();

            if (!pair.GetLeft()->HasIntSegs()) {

                if (firstPairInCycle) {

                    // Check, if PFace left is outside or inside 
                    // of the other region unit
                    // and add intSegs depending of the operation.
                    
                    if (partner->IsEntirelyOutside(pair.GetLeft())) {
                        
                        pair.GetLeft()->MarkAsEntirelyOutside();

                        if (op == UNION || 
                           (op == MINUS && pair.GetLeft()->IsPartOfUnitA())) {

                            pair.AddEntireBoundary();

                        } else { 
                            
                            // op == INTERSECTION || 
                            // (op == MINUS && pair.GetLeft()->IsPartOfUnitB()):
                            // nothing to do.
                        }

                    } else { // The PFace is entirely inside of unit B.
                        
                        pair.GetLeft()->MarkAsEntirelyInside();

                        if (op == INTERSECTION || 
                           (op == MINUS && pair.GetLeft()->IsPartOfUnitB())) {

                            pair.AddEntireBoundary();

                        } else { 
                            
                            // op == UNION || 
                            // (op == MINUS && pair.GetLeft()->IsPartOfUnitA()):
                            // nothing to do.
                        }

                    }

                } else { // not the first pair in cycle.

                    // nothing to do.
                }

            } else { // pair.GetLeft()->HasIntSegs() == true

                pair.ComputeBoundarySegments();
            }

            firstPairInCycle = false;
        }
    }
}

void PUnit::FindMates() {

    for (list<PFaceCycle>::iterator iter = pFacesCycles.begin(); iter
            != pFacesCycles.end(); iter++) {

        PFaceCycleIterator pIter = (*iter).GetPFaceCycleIterator();

        while (pIter.HasNext()) {

            pIter.GetNext()->FindMates();
        }
    }
}

void PUnit::CollectIntSegs(PUnitPair* target) {

    for (list<PFaceCycle>::iterator iter = pFacesCycles.begin(); iter
            != pFacesCycles.end(); iter++) {

        PFaceCycleIterator pIter = (*iter).GetPFaceCycleIterator();

        while (pIter.HasNext()) {

            pIter.GetNext()->CollectIntSegs(target);
        }
    }
}

void PUnit::FindMatesAndCollectIntSegs(PUnitPair* target) {

    for (list<PFaceCycle>::iterator iter = pFacesCycles.begin(); iter
            != pFacesCycles.end(); iter++) {

        PFaceCycleIterator pIter = (*iter).GetPFaceCycleIterator();

        while (pIter.HasNext()) {

            PFace* pFace = pIter.GetNext();
            pFace->FindMates();
            pFace->CollectIntSegs(target);
        }
    }
}

bool PUnit::IsEntirelyOutside(const PFace* pFace) {

    // Check, if PFace's location is already known:
    
    if (pFace->IsEntirelyOutside())
        return true;
    
    if (pFace->IsEntirelyInside())
        return false;
    
    // We don't know and have to use the plumbline algorithm
    // to find the answer:
    
    cout << "Plumbline used." << endl;
    
    //Vector3D aToMiddle = (pFace->GetC_XYT() - pFace->GetA_XYT()) * 0.5;
    Point3D middleXYT = (pFace->GetA_XYT() + pFace->GetC_XYT()) * 0.5;
    Point middleXY(middleXYT.GetX(), middleXYT.GetY());
    
    cout << "middleXYT: " << middleXYT.GetX() << " | " 
                          << middleXYT.GetY() << " | " 
                          << middleXYT.GetT() << endl;
    
    return !GetMedianRegion().InnerContains(middleXY);
}



PUnitPair::PUnitPair(const URegionEmb* _uRegionA, 
                     const DBArray<MSegmentData>* aArray, 
                     const URegionEmb* _uRegionB, 
                     const DBArray<MSegmentData>* bArray) :
                         
    unitA(true, _uRegionA, aArray, this), 
    unitB(false, _uRegionB, bArray, this) {

    unitA.SetPartner(&unitB);
    unitB.SetPartner(&unitA);
    
    ComputeOverlapRect();
    
    result = new vector<URegion>();
}

vector<URegion>* PUnitPair::Intersection() {
    
    cout << "Welcome to PUnitPair::Intersection()" << endl;

    if (HasOverlappingBoundingRect()) {

        cout << "HasOverlappingBoundingRect() == true" << endl;
        
        CreatePFaces();
        //PrintPFaceCycles();
        //PrintPFacePairs();
        ComputeInnerIntSegs(INTERSECTION);
        //PrintIntSegsOfPFaces();
        AddBoundarySegments(INTERSECTION);
        //PrintIntSegsOfPFaces();
        //FindMates();
        //PrintIntSegsOfPFaces();
        FindMatesAndCollectIntSegs();
        //PrintIntSegsOfGlobalList();
        //PrintIntSegsOfGlobalListAsVRML();
        PrintUnitsAsVRML();
        //ConstructResultURegions();

    } else {

        cout << "HasOverlappingBoundingRect() == false" << endl;

        CreatePFaces();
        //PrintPFaceCycles();
        //PrintPFacePairs();
        ComputeInnerIntSegs(INTERSECTION);
        //PrintIntSegsOfPFaces();
        AddBoundarySegments(INTERSECTION);
        //PrintIntSegsOfPFaces();
        //FindMates();
        //PrintIntSegsOfPFaces();
        FindMatesAndCollectIntSegs();
        //PrintIntSegsOfGlobalList();
        //PrintIntSegsOfGlobalListAsVRML();
        PrintUnitsAsVRML();
        //ConstructResultURegions();

        
        // construct one empty uregion:
        // URegionEmb* ure = new URegionEmb(this->GetTimeInterval(), 0);
        // URegion ur(0u);
        // ur.SetEmbedded(ure);
        
        // result->push_back(ur);
        
    }
    
    return result;
}

vector<URegion>* PUnitPair::Union() {

    cout << "Welcome to PUnitPair::Union()" << endl;

    if (HasOverlappingBoundingRect()) {

        cout << "HasOverlappingBoundingRect() == true" << endl;

        CreatePFaces();
        //PrintPFaceCycles();
        //PrintPFacePairs();
        ComputeInnerIntSegs(UNION);
        //PrintIntSegsOfPFaces();
        AddBoundarySegments(UNION);
        //PrintIntSegsOfPFaces();
        //FindMates();
        //PrintIntSegsOfPFaces();
        FindMatesAndCollectIntSegs();
        //PrintIntSegsOfGlobalList();
        //PrintIntSegsOfGlobalListAsVRML();
        PrintUnitsAsVRML();
        //ConstructResultURegions();

    } else {

        cout << "HasOverlappingBoundingRect() == false" << endl;

        CreatePFaces();
        //PrintPFaceCycles();
        //PrintPFacePairs();
        ComputeInnerIntSegs(UNION);
        //PrintIntSegsOfPFaces();
        AddBoundarySegments(UNION);
        //PrintIntSegsOfPFaces();
        //FindMates();
        //PrintIntSegsOfPFaces();
        FindMatesAndCollectIntSegs();
        //PrintIntSegsOfGlobalList();
        //PrintIntSegsOfGlobalListAsVRML();
        PrintUnitsAsVRML();
        //ConstructResultURegions();

        // TODO: construct one uregion with all elements
        // from both, unitA and unitB.
    }

    return result;
}

vector<URegion>* PUnitPair::Minus() {

    cout << "Welcome to PUnitPair::Minus()" << endl;

    if (HasOverlappingBoundingRect()) {

        cout << "HasOverlappingBoundingRect() == true" << endl;

        CreatePFaces();
        //PrintPFaceCycles();
        //PrintPFacePairs();
        ComputeInnerIntSegs(MINUS);
        //PrintIntSegsOfPFaces();
        AddBoundarySegments(MINUS);
        //PrintIntSegsOfPFaces();
        //FindMates();
        //PrintIntSegsOfPFaces();
        FindMatesAndCollectIntSegs();
        //CollectIntSegs();
        //PrintIntSegsOfGlobalList();
        //PrintIntSegsOfGlobalListAsVRML();
        PrintUnitsAsVRML();
        //ConstructResultURegions();

    } else {

        cout << "HasOverlappingBoundingRect() == false" << endl;

        CreatePFaces();
        //PrintPFaceCycles();
        //PrintPFacePairs();
        ComputeInnerIntSegs(MINUS);
        //PrintIntSegsOfPFaces();
        AddBoundarySegments(MINUS);
        //PrintIntSegsOfPFaces();
        //FindMates();
        //PrintIntSegsOfPFaces();
        FindMatesAndCollectIntSegs();
        //PrintIntSegsOfGlobalList();
        //PrintIntSegsOfGlobalListAsVRML();
        PrintUnitsAsVRML();
        //ConstructResultURegions();


        // construct one uregion identical to unitA:
        //URegion ur(unitA.GetNoOfPFaces());
        //ur.SetEmbedded(unitA.GetURegionEmb());

        //result->push_back(ur);
    }
    
    return result;
}
