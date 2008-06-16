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

void PUnitPair::ComputeOverlapRect() {
    
    overlapRect = unitA.GetBoundingRect().Intersection(unitB.GetBoundingRect());
}

void PUnitPair::CreatePFaces() {
    
    unitA.CreatePFaces();
    unitB.CreatePFaces();
}

void PUnitPair::ComputeInnerIntSegs(const SetOp op) {

    for (list<PFace*>::iterator iterA = unitA.pFacesReduced.begin(); iterA
            != unitA.pFacesReduced.end(); iterA++) {

        for (list<PFace*>::iterator iterB = unitB.pFacesReduced.begin(); iterB
                != unitB.pFacesReduced.end(); iterB++) {

            (*iterA)->Intersection(**iterB, op);
        }
    }
}

void PUnitPair::AddBoundarySegments(const SetOp op) {

    unitA.AddBoundarySegments(op);
    unitB.AddBoundarySegments(op);
}

void PUnitPair::FindMates() {

    unitA.FindMates();
    unitB.FindMates();
}

void PUnitPair::CollectIntSegs() {

    unitA.CollectIntSegs(this);
    unitB.CollectIntSegs(this);
}

void PUnitPair::FindMatesAndCollectIntSegs() {
    
    unitA.FindMatesAndCollectIntSegs(this);
    unitB.FindMatesAndCollectIntSegs(this);
}

void PUnitPair::ConstructResultURegions() {

    list<IntersectionSegment*> activeIntSegList;
    list<IntersectionSegment*>::iterator activeSegIter;
    
    list<IntersectionSegment*> movingSegList;
    list<IntersectionSegment*>::iterator movingSegIter;

    set<IntersectionSegment*>::iterator iter = globalIntSegSet.begin();
    
    // Append to activeIntSegList all intersection segments with 
    // segment.GetStartT() == this->GetStartTime():
    
    double t = this->GetStartTime();
    
    while (iter != globalIntSegSet.end() && 
            NumericUtil::NearlyEqual((*iter)->GetStartT(), t)) {

        activeIntSegList.push_back(*iter);
        iter++;
    }
    
    

    while (iter != globalIntSegSet.end()) { // For each relevant timevalue...

        t = (*iter)->GetStartT();

        // Append to activeIntSegList all segments with segment.GetStartT() == t

        while (iter != globalIntSegSet.end() && 
                NumericUtil::NearlyEqual((*iter)->GetStartT(), t)) {

            activeIntSegList.push_back(*iter);
            iter++;
        }
        
        // For each segment in activeIntSegList...
        activeSegIter = activeIntSegList.begin();
        while (activeSegIter != activeIntSegList.end()) {  
            
            IntersectionSegment* activeMate = 
                (*activeSegIter)->GetActiveLeftMate();
        }
    }
}

void PUnitPair::PrintPFaceCycles() {

    cout << endl;
    cout << "*********************************************" << endl;
    cout << "PFaces of Unit A:" << endl;
    cout << "*********************************************" << endl;
    cout << endl;

    unitA.PrintPFaceCycles();

    cout << endl;
    cout << "*********************************************" << endl;
    cout << "PFaces of Unit B:" << endl;
    cout << "*********************************************" << endl;
    cout << endl;

    unitB.PrintPFaceCycles();

}

void PUnitPair::PrintPFacePairs() {

    cout << endl;
    cout << "*********************************************" << endl;
    cout << "PFacePairs of Unit A:" << endl;
    cout << "*********************************************" << endl;
    cout << endl;

    unitA.PrintPFacePairs();

    cout << endl;
    cout << "*********************************************" << endl;
    cout << "PFacePairs of Unit B:" << endl;
    cout << "*********************************************" << endl;
    cout << endl;

    unitB.PrintPFacePairs();

}

void PUnitPair::PrintIntSegsOfPFaces() {
    
    cout << endl;
        cout << "*********************************************" << endl;
        cout << "IntSegs of Unit A:" << endl;
        cout << "*********************************************" << endl;
        cout << endl;

        unitA.PrintIntSegsOfPFaces();

        cout << endl;
        cout << "*********************************************" << endl;
        cout << "IntSegs of Unit B:" << endl;
        cout << "*********************************************" << endl;
        cout << endl;

        unitB.PrintIntSegsOfPFaces();
}

void PUnitPair::PrintIntSegsOfGlobalList() {

    cout << "*********************************************" << endl;
    cout << "globalIntSegSet:" << endl;
    cout << globalIntSegSet.size() << " segments" << endl;
    cout << "*********************************************" << endl;
    cout << endl;
    
    multiset<IntersectionSegment*>::iterator iter;
    for (iter = globalIntSegSet.begin(); iter != globalIntSegSet.end();++iter) {

        (*iter)->Print();
        cout << endl;
    }
}

void PUnitPair::PrintIntSegsOfGlobalListAsVRML(ofstream& target, 
                                               const string& color) {

    const double scale = VRML_SCALE_FACTOR;
    
    target << "Transform {" << endl;
    target << "\tscale " << scale << " " << scale << " " << scale << endl;
    target << "\tchildren [" << endl;
    target << "\t\tShape {" << endl;
    target << "\t\t\tgeometry IndexedLineSet {" << endl;
    target << "\t\t\t\tcoord Coordinate {" << endl;
    target << "\t\t\t\t\tpoint [" << endl;

    multiset<IntersectionSegment*>::iterator iter;
    for (iter = globalIntSegSet.begin(); iter != globalIntSegSet.end();++iter) {

        target << "\t\t\t\t\t\t" << (*iter)->GetVRMLDesc() << endl;
    }

    target << "\t\t\t\t\t]  # end point" << endl;
    target << "\t\t\t\t} # end coord" << endl;
    target << "\t\t\t\tcoordIndex [" << endl;
    
    const int noPoints = globalIntSegSet.size() * 2;

    for (int i = 0; i < noPoints; i += 2) {

        target << "\t\t\t\t\t" << i << ", " << i + 1 << ", " << "-1," << endl;
    }

    target << "\t\t\t\t] # end coordIndex" << endl;
    
    target << "\t\t\t\tcolor Color { color [ " << color << " ] }" << endl;
    
    target << "\t\t\t} # end geometry" << endl;
    target << "\t\t} # end shape" << endl;
    target << "\t] # end children" << endl;
    target << "} # end Transform" << endl;
}

void PUnitPair::PrintUnitsAsVRML() {
    
    const string colorA = "1 1 0";
    const string colorB = "0 0.6 1";
    const string colorIntSegs = "1 0 0";
    
    ofstream target(VRML_OUTFILE);
    
    if (!target.is_open()) {
        
        cerr << "Unable to open file: " << VRML_OUTFILE << endl;
        return;
    }
        
    target << "#VRML V2.0 utf8" << endl;
    
    target << endl;
    target << "# Unit A:" << endl;
    target << endl;
    unitA.PrintVRMLDesc(target, colorA);
    
    target << endl;
    target << "# Unit B:" << endl;
    target << endl;
    unitB.PrintVRMLDesc(target, colorB);
    
    target << endl;
    target << "# Intersection segments of global list:" << endl;
    target << endl;
    this->PrintIntSegsOfGlobalListAsVRML(target, colorIntSegs);
    
    target.close();
}

pair<IntersectionSegment*, IntersectionSegment*> 
IntersectionSegment::createBuddyPair(const Point3D& a, const Point3D& b) {

    IntersectionSegment* first = new IntersectionSegment();
    IntersectionSegment* second = new IntersectionSegment();

    // The startpoint's t-coord is always lower or equal to the
    // endpoint's t-coord.
    // Note: We don't care for x and y!

    if (a.GetT() <= b.GetT()) {

        first->startXYT = second->startXYT = new Point3D(a);
        first->endXYT = second->endXYT = new Point3D(b);

    } else { // a.GetT() > b.GetT()

        first->startXYT = second->startXYT = new Point3D(b);
        first->endXYT = second->endXYT = new Point3D(a);
    }

    first->matesOfThis = new deque<IntersectionSegment*>();
    second->matesOfBuddy = first->matesOfThis;
    
    second->matesOfThis = new deque<IntersectionSegment*>();
    first->matesOfBuddy = second->matesOfThis;
            
    first->inserted = second->inserted = new bool(false);
    first->hasReference = second->hasReference = new bool(true);

    return pair<IntersectionSegment*, IntersectionSegment*>(first, second);
}

bool IntersectionSegment::LessByStartWEndW(IntersectionSegment* s) const {

        if (NumericUtil::Lower(this->GetStartW(), s->GetStartW()))
            return true;
        else 
            if (NumericUtil::Greater(this->GetStartW(), s->GetStartW()))
                return false;
            else // this->GetStartW() == s->GetStartW()         
                if (NumericUtil::Lower(this->GetEndW(), s->GetEndW()))
                    return true;
                else 
                    if (NumericUtil::Greater(this->GetEndW(), s->GetEndW()))
                        return false;
                    else // this->GetEndW() == s->GetEndW() 
                        return this->GetStartWT().IsLeft(s->GetStartWT(), 
                                                         s->GetEndWT());    
}

void IntersectionSegment::SetWCoords(const PFace& pFace) {

    this->SetStartW(pFace.TransformToW(*this->GetStartXYT()));
    this->SetEndW(pFace.TransformToW(*this->GetEndXYT()));
}

void IntersectionSegment::SetSideOfResultFace(const PFace& self, 
                                              const PFace& other, 
                                              const SetOp op) {

    // First, we compute the normalized cross product of the normal vectors of 
    // PFaces self and other:
    Vector3D cross = self.GetNormalVector() ^ other.GetNormalVector();
    cross.Normalize();
    
    // Second, we compute the normalized vector from the startpoint to
    // the endpoint of this intersection segment:
    Vector3D startToEnd = *GetEndXYT() - *GetStartXYT();
    startToEnd.Normalize();
    
    // Third, we calculate the cosinus of the angle between cross and startToEnd
    // Note that both vectors have unit length and the angle 
    // must be either 0 or pi. Hence the cosinus equals 1 or -1.
    double cos = startToEnd * cross;
    
    //cout << "cos = " << cos << endl;
    assert(NumericUtil::NearlyEqual(cos, 1.0) || 
           NumericUtil::NearlyEqual(cos, -1.0));
    
    // If cos equals 1, 
    // the normal vector of other points to the right side of this segment.
    // If cos equals -1, 
    // the normal vector of other points to the left side of this segment.

    if (op == INTERSECTION || (op == MINUS && other.IsPartOfUnitA())) {

        if (cos > 0.0) // normalOfOther points to the right side of startToEnd.
            this->SetResultFaceIsLeft();
        else
            // normalOfOther points to the left side of startToEnd.
            this->SetResultFaceIsRight();

    } else { // op == UNION || (op == MINUS && self.IsPartOfUnitA())

        if (cos > 0.0) // normalOfOther points to the right side of startToEnd.
            this->SetResultFaceIsRight();
        else
            // normalOfOther points to the left side of startToEnd.
            this->SetResultFaceIsLeft();
    }
}


void IntersectionSegment::Print() {
    
    
    cout << "Start_XYT: " << GetStartXYT()->GetX() << " | "
                          << GetStartXYT()->GetY() << " | "
                          << GetStartXYT()->GetT() << endl;
    
    cout << "End_XYT: " << GetEndXYT()->GetX() << " | "
                        << GetEndXYT()->GetY() << " | "
                        << GetEndXYT()->GetT() << endl;
                   
    
    //cout << "Start_WT: " << GetStartW() << " | " << GetStartT() << endl;
    //cout << "End_WT: " << GetEndW() << " | " << GetEndT() << endl;
    
    //cout << "IsRightBoundary() == " << this->IsRightBoundary() << endl;
    
    //cout << "ResultFaceIsLeft: " << ResultFaceIsLeft() << endl;
    //cout << "RelationToRightBoundary: " 
    // << GetRelationToRightBoundaryOfPFace() << endl;
    
    //cout << "inserted:" << *inserted << endl;
    //cout << "matesOfThis->size() == " << matesOfThis->size() << endl;
    
        
}

string IntersectionSegment::GetVRMLDesc() {
    
    return GetStartXYT()->GetVRMLDesc() + GetEndXYT()->GetVRMLDesc();
}

void IntersectionSegment::PrintMates() {
    
    //cout << "Welcome to IntersectionSegment::PrintMates()" << endl;
    
    //cout << "inserted:" << *inserted << endl;
    //cout << "matesOfThis->size() == " << matesOfThis->size() << endl;
    
    if (matesOfThis->empty()) {
        
        cout << "matesOfThis is empty." << endl;
        cout << "*********************************************" << endl;
        return;
    }
    
    deque<IntersectionSegment*> mates = *matesOfThis;
    
    deque<IntersectionSegment*>::iterator iter;
    for (iter = mates.begin(); iter != mates.end(); ++iter) {
        
        (*iter)->Print();
    }
    
}

bool GeneralIntSegSetCompare::operator()(const IntersectionSegment* const& s1,
        const IntersectionSegment* const& s2) const {
    
    // We sort by (t_start, w_start, w_end, IsLeft())

    // Precondition: s1->GetStartT() < s1->GetEndT() && 
    //               s2->GetStartT() < s2->GetEndT()

    if (NumericUtil::Lower(s1->GetStartT(), s2->GetStartT()))
        return true;
    else
        if (NumericUtil::Greater(s1->GetStartT(), s2->GetStartT()))
            return false;
        else // s1->GetStartT() == s2->GetStartT()
            if (NumericUtil::Lower(s1->GetStartW(), s2->GetStartW()))
                return true;
            else
                if (NumericUtil::Greater(s1->GetStartW(), s2->GetStartW()))
                    return false;
                else // s1->GetStartW() == s2->GetStartW()
                    if (NumericUtil::Lower(s1->GetEndW(), s2->GetEndW()))
                        return true;
                    else
                        if (NumericUtil::Greater(s1->GetEndW(), s2->GetEndW()))
                            return false;
                        else  // s1->GetEndW() == s2->GetEndW()
                            return s1->GetStartWT().IsLeft(s2->GetStartWT(), 
                                                           s2->GetEndWT());
}

bool RightIntSegSetCompare::operator()(const IntersectionSegment* const& s1,
        const IntersectionSegment* const& s2) const {

    assert(s1->GetRelationToRightBoundaryOfPFace() == TOUCH_IN_STARTPOINT || 
           s1->GetRelationToRightBoundaryOfPFace() == TOUCH_IN_ENDPOINT);
    
    assert(s2->GetRelationToRightBoundaryOfPFace() == TOUCH_IN_STARTPOINT || 
           s2->GetRelationToRightBoundaryOfPFace() == TOUCH_IN_ENDPOINT);
    
    double t1, t2, t3, t4;

    if (s1->GetRelationToRightBoundaryOfPFace() == TOUCH_IN_STARTPOINT) {

        t1 = s1->GetStartT();
        t2 = s1->GetEndT();

    } else { // TOUCH_IN_ENDPOINT

        t1 = s1->GetEndT();
        t2 = s1->GetStartT();
    }

    if (s2->GetRelationToRightBoundaryOfPFace() == TOUCH_IN_STARTPOINT) {

        t3 = s2->GetStartT();
        t4 = s2->GetEndT();

    } else { // TOUCH_IN_ENDPOINT

        t3 = s2->GetEndT();
        t4 = s2->GetStartT();
    }

    if (NumericUtil::NearlyEqual(t1, t3))
        return NumericUtil::Lower(t2, t4);
    else
        return NumericUtil::Lower(t1, t3);
}

PFace::PFace(PUnit* _unit, 
             const MSegmentData* _mSeg, 
             bool _initialStartPointIsA) :
                 
    unit(_unit), mSeg(_mSeg), initialStartPointIsA(_initialStartPointIsA) {

    hasIntSegs = false;
    hasBoundaryIntSegs = false;
    entirelyInsideOutside = UNKNOWN;
    intSegMaxW = 0;
    
    ComputeBoundingRect();
    ComputeNormalVector();
    ComputeWTCoords();
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

    //if (NumericUtil::Lower(s, 0.0) || NumericUtil::Lower(1.0, s)) 
    // No intersection point, if s < -eps or s > 1 + eps.
    if (s < 0.0 || s > 1.0)
        return false;

    // Compute segment intersection point:
    result = seg.GetStart() + s * u;

    return true;
}

bool PFace::Intersection(PFace& other, const SetOp op) {
    
    if (!this->GetBoundingRect().Intersects(other.GetBoundingRect()))
        return false;

    if (this->IsParallelTo(other)) // The PFaces are parallel.
        return false;

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
        return false;

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

    pair<IntersectionSegment*, IntersectionSegment*> result =
            intPointSet.GetIntersectionSegment();

    if (result.first == 0 || result.second == 0) // There is no intersection.
        return false;

    // We got an intersection segment buddy pair
    // and compute the w-coords of each segment:
    result.first->SetWCoords(*this);
    result.second->SetWCoords(other);
    
    // Check, on which side of the segment the result face is:
    result.first->SetSideOfResultFace(*this, other, op);
    result.second->SetSideOfResultFace(other, *this, op);
    
    // Add one of them to each PFace:
    this->AddInnerIntSeg(result.first);
    other.AddInnerIntSeg(result.second);

    return true;
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

    boundingRect = Rectangle<2>(true, x.first, x.second, y.first, y.second);
}

void PFace::ComputeWTCoords() {

    // The vector w is the normalized exterior cross product 
    // of the t-unit-vector and the normal vector of the PFace:

    // wVector = Vector3D(0.0, 0.0, 1.0) ^ GetNormalVector();
    // wVector.Normalize();

    // This can be simplified to:
    wVector = Vector3D(-GetNormalVector().GetY(), 
                        GetNormalVector().GetX(), 
                        0.0);
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

void PFace::DeleteOrthogonalIntSegs() {

    list<IntersectionSegment*>::iterator iter;
    for (iter = orthogonalIntSegList.begin(); iter
            != orthogonalIntSegList.end(); ++iter)
        (*iter)->Delete();
}

void PFace::DeleteGeneralIntSegs() {

    set<IntersectionSegment*>::iterator iter;
    for (iter = generalIntSegSet.begin(); iter != generalIntSegSet.end();++iter)
        (*iter)->Delete();
}

void PFace::FindMates() {

    MateEngine mateEngine(&generalIntSegSet);
    set<double>::iterator timeIter;
    int count = 0;
    // Find mates for each relevant time value:

    for (timeIter = relevantTimeValueSet.begin(); timeIter
            != relevantTimeValueSet.end(); timeIter++) {

        //cout << count << endl;
        cout << *timeIter << endl;
        cout << "relevantTimeValueSet.size() == "
                << relevantTimeValueSet.size() << endl;
        mateEngine.ComputeTimeLevel(*timeIter);

        //if (++count == 10)
        //break;
    }

}

void PFace::CollectIntSegs(PUnitPair* target) const {

    set<IntersectionSegment*>::iterator iter;
    for (iter = generalIntSegSet.begin(); iter != generalIntSegSet.end();++iter)
        target->AddIntSeg(*iter);
}

bool PFace::IsParallelTo(const PFace& pf) const {

    Vector3D cross = GetNormalVector() ^ pf.GetNormalVector();

    return NumericUtil::NearlyEqual(cross.GetX(), 0.0)
            && NumericUtil::NearlyEqual(cross.GetY(), 0.0)
            && NumericUtil::NearlyEqual(cross.GetT(), 0.0);
}

void PFace::AddInnerIntSeg(IntersectionSegment* intSeg) {

    // Precondition: w-coords and resultFaceIsLeft are already set.

    if (IntersectsRightBoundary(intSeg)) {

        rightIntSegSet.insert(intSeg);
    }

    if (!intSeg->IsOrthogonalToTAxis()) {

        generalIntSegSet.insert(intSeg);

    } else {

        orthogonalIntSegList.push_back(intSeg);
    }

    relevantTimeValueSet.insert(intSeg->GetStartT());
    relevantTimeValueSet.insert(intSeg->GetEndT());

    hasIntSegs = true;
    hasInnerIntSegs = true;
}

void PFace::AddBoundaryIntSegs(IntersectionSegment* intSeg) {

    // Set the w-coords of the intersection segment, 
    // depending of this PFace:
    intSeg->SetStartW(this->TransformToW(*intSeg->GetStartXYT()));
    intSeg->SetEndW(this->TransformToW(*intSeg->GetEndXYT()));

    generalIntSegSet.insert(intSeg);
    relevantTimeValueSet.insert(intSeg->GetStartT());
    relevantTimeValueSet.insert(intSeg->GetEndT());

    hasIntSegs = true;
    hasBoundaryIntSegs = true;
}
