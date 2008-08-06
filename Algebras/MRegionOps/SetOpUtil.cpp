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

using namespace std;

namespace mregionops {



/*

Class Point3DExt

*/

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

/*

Class IntersectionSegment

*/

unsigned int IntersectionSegment::instancePairCount = 0;

pair<IntersectionSegment*, IntersectionSegment*> 
IntersectionSegment::createBuddyPair(const Point3D& a, 
                                     const Point3D& b) {

    IntersectionSegment* first = new IntersectionSegment();
    IntersectionSegment* second = new IntersectionSegment();
    
    first->SetBuddy(second);
    second->SetBuddy(first);

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
    
    first->inserted = second->inserted = new bool(false);
    first->hasReference = second->hasReference = new bool(true);
    
    //cout << "Create IntSegPair: " 
         //<< IntersectionSegment::instancePairCount << endl;
    
    first->id = second->id = 
        new unsigned int(IntersectionSegment::instancePairCount++);
    
    return pair<IntersectionSegment*, IntersectionSegment*>(first, second);
}

bool IntersectionSegment::IsLeftOf(const IntersectionSegment* intSeg) const {
    
    // TODO: Avoid multiple computation of the same value.
    
    Segment2D s(intSeg->GetStartWT(), intSeg->GetEndWT(), false);

    if (this->GetStartWT().IsLeft(s))
        return true;

    if (this->GetStartWT().IsRight(s))
        return false;
    // Startpoint of this is colinear to intSeg.
    if (this->GetEndWT().IsLeft(s))
        return true;

    if (this->GetEndWT().IsRight(s))
        return false;

    return true;
    //else // this is colinear to intSeg.
    //if (op == INTERSECTION)
    //return this->ResultFaceIsLeft();
    //else // op == UNION | MINUS
    //return this->ResultFaceIsRight();    
}

void IntersectionSegment::SetWCoords() {

    SetStartW(GetPFace()->TransformToW(*GetStartXYT()));
    SetEndW(GetPFace()->TransformToW(*GetEndXYT()));
}

void IntersectionSegment::SetSideOfResultFace(const PFace& self,
                                              const PFace& other) {
    
    const SetOp op = self.GetOperation();

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
    assert(NumericUtil::NearlyEqual(cos, 1.0) || NumericUtil::NearlyEqual(cos,
            -1.0));

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

void IntersectionSegment::SetRelationToBoundaryOfPFace() {

    if (this->GetRelationToBoundaryOfPFace() != UNDEFINED_RELATION)
        return;

    const Segment2D rightBoundary(GetPFace()->GetB_WT(), 
                                  GetPFace()->GetD_WT(), 
                                  false);
    
    const Segment2D leftBoundary(GetPFace()->GetA_WT(), 
                                 GetPFace()->GetC_WT(), 
                                 false);

    const bool startOnRightBoundary = GetStartWT().IsColinear(rightBoundary);
    const bool endOnRightBoundary = GetEndWT().IsColinear(rightBoundary);
    const bool startOnLeftBoundary = GetStartWT().IsColinear(leftBoundary);
    const bool endOnLeftBoundary = GetEndWT().IsColinear(leftBoundary);
    
    if (!startOnRightBoundary && !endOnRightBoundary &&
        !startOnLeftBoundary && !endOnLeftBoundary) {
        
        SetRelationToBoundaryOfPFace(NO_TOUCH_OF_BOUNDARY);
        return;
    }
    
    if (startOnRightBoundary && !endOnRightBoundary) {

        SetRelationToBoundaryOfPFace(TOUCH_RIGHT_BOUNDARY_IN_STARTPOINT_ONLY);
        return;
    }
    
    if (!startOnRightBoundary && endOnRightBoundary) {

        SetRelationToBoundaryOfPFace(TOUCH_RIGHT_BOUNDARY_IN_ENDPOINT_ONLY);
        return;
    }

    if (startOnRightBoundary && endOnRightBoundary) {

        SetRelationToBoundaryOfPFace(COLINEAR_TO_RIGHT_BOUNDARY);
        return;
    }

    if (startOnLeftBoundary && endOnLeftBoundary) {
        
        SetRelationToBoundaryOfPFace(COLINEAR_TO_LEFT_BOUNDARY);
        return;
    }
    
    SetRelationToBoundaryOfPFace(TOUCH_LEFT_BOUNDARY_ONLY);
}

Point3D IntersectionSegment::Evaluate(const double t) const {

    // We compute the intersection point
    // of the horizontal plane, defined by t, and this segment.
    
    // Precondition:
    // t is between t_start and t_end.
    assert(NumericUtil::Between(this->GetStartT(), t, this->GetEndT()));

    // Point3D pointInPlane(0.0, 0.0, t);
    // Vector3D normalVectorOfPlane(0.0, 0.0, 1.0);
    // Vector3D u = *this->GetEndXYT() - *this->GetStartXYT();
    // Vector3D w = this->GetStartXYT() - pointInPlane;
    // double d = normalVectorOfPlane * u;
    // double n = -normalVectorOfPlane * w;
    
    // This can be simplified to:

    const Vector3D u = *this->GetEndXYT() - *this->GetStartXYT();
    const double d = this->GetEndT() - this->GetStartT();
    const double n = t - this->GetStartT();

    // This segment must not be parallel to plane:
    assert(!NumericUtil::NearlyEqual(d, 0.0));

    const double s = n / d;

    // This segment intersects the plane:
    assert(NumericUtil::Between(0.0, s, 1.0));

    // Compute segment intersection point:
    return *this->GetStartXYT() + s * u;
}

Vector3D IntersectionSegment::GetNormalVectorOfResultFace() const {

    return pFace->GetNormalVectorOfResultFace();
}

pair<IntersectionSegment*, const PFace*> 
IntersectionSegment::GetNextActiveMateOfResultCycle(
        const IntersectionSegment* prev) const {

    IntersectionSegment* mate1 = this->mates.GetActiveMate();

    if (prev != 0) {

        if (mate1->GetID() != prev->GetID()) {

            return make_pair(mate1, this->GetPFace());

        } else {

            IntersectionSegment* mate2 = buddy->mates.GetActiveMate();
            assert(mate2->GetID() != prev->GetID());

            return make_pair(mate2, buddy->GetPFace());
        }

    } else { // prev == 0

        // This is first segment in cycle.
        return make_pair(mate1, this->GetPFace());
    }
}

bool IntersectionSegment::IsTouchingLine(const PFace& other) const {
    
    if (!IsColinearToBoundaryOfPFace())
        return false;
    
    const Vector2D wOther(other.GetWVector());
    Vector2D wSelf;
    Vector2D wNeighbour;
    
    if (relationToBoundary == COLINEAR_TO_RIGHT_BOUNDARY) {
        
        wSelf = Vector2D(-(GetPFace()->GetWVector()));
        wNeighbour = Vector2D(GetPFace()->GetRightNeighbour()->GetWVector());
        
    } else { // relationToBoundary == COLINEAR_TO_LEFT_BOUNDARY
        
        wSelf = Vector2D(GetPFace()->GetWVector());
        wNeighbour = Vector2D(-(GetPFace()->GetLeftNeighbour()->GetWVector()));
    }
    
    // Now, we check if wSelf and wNeighbour points to the same side of wOther:
    const Point2D pSelf(wSelf.GetX(), wSelf.GetY());
    const Point2D pNeighbour(wNeighbour.GetX(), wNeighbour.GetY());
    const Point2D pOther(wOther.GetX(), wOther.GetY());
    const Point2D zero;
    
    const double sideOfSelf = pSelf.WhichSide(zero, pOther);
    const double sideOfNeighbour = pNeighbour.WhichSide(zero, pOther);
    
    const double sgn = sideOfSelf * sideOfNeighbour;
    
    return NumericUtil::Greater(sgn, 0.0);
}

void IntersectionSegment::Print() const {
    
    cout << "ID: " << GetID() << endl;

    cout << "Start_XYT: " << GetStartXYT()->GetX() << " | " 
                          << GetStartXYT()->GetY() << " | " 
                          << GetStartXYT()->GetT() << endl;

    cout << "End_XYT: " << GetEndXYT()->GetX() << " | " 
                        << GetEndXYT()->GetY() << " | " 
                        << GetEndXYT()->GetT() << endl;

    //cout << "Start_WT: " << GetStartW() << " | " << GetStartT() << endl;
    //cout << "End_WT: " << GetEndW() << " | " << GetEndT() << endl;

    cout << "IsRightBoundary() == " << this->IsRightBoundary() << endl;

    cout << "ResultFaceIsLeft: " << ResultFaceIsLeft() << endl;
    cout << "RelationToBoundary: " 
    << GetRelationToBoundaryOfPFace() << endl;
    cout << "ResultFaceIsVanished: " << ResultFaceIsVanished() << endl;

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
    
    if (mates.IsEmpty()) {
        
        cout << "matesOfThis is empty." << endl;
        cout << "*********************************************" << endl;
        
    } else {
        
        mates.Print();
    }
}



/*

 Class MedianCycleSeg

*/

MedianCycleSeg::MedianCycleSeg(const Segment2D& _seg,
                               const PFace* _pFace,
                               ResultMovingCycle* _cycle) :

      seg(_seg),
      pFace(_pFace),
      cycle(_cycle) {
    
}

bool MedianCycleSeg::SameCycle(const MedianCycleSeg* mcs) const {

    return cycle->GetID() == mcs->cycle->GetID();
}

bool MedianCycleSeg::IsAbove(const MedianCycleSeg* mcs) const {

    // Precondition: Both segments are currently active during a
    // plane-sweep in x-direction.

    const double a = seg.GetStart().WhichSide(mcs->seg);
    const double b = seg.GetEnd().WhichSide(mcs->seg);

    if (NumericUtil::GreaterOrNearlyEqual(a, 0.0)
            && NumericUtil::GreaterOrNearlyEqual(b, 0.0)) {

        // Both, start- and endpoint of this segment are
        // left or onto the other segment.
        return true;
    }

    if (NumericUtil::LowerOrNearlyEqual(a, 0.0)
            && NumericUtil::LowerOrNearlyEqual(b, 0.0)) {

        // Both, start- and endpoint of this segment are
        // right or onto the other segment.
        return false;
    }

    // Start and endpoint of this segment are on different
    // sides of the other segment.
    // We have to check the opposite direction:

    const double c = mcs->seg.GetStart().WhichSide(seg);
    const double d = mcs->seg.GetEnd().WhichSide(seg);

    return NumericUtil::LowerOrNearlyEqual(c, 0.0)
            && NumericUtil::LowerOrNearlyEqual(d, 0.0);
}

/*

 Class MedianCycle

*/

void MedianCycle::Append(const Segment2D& seg, const PFace* pFace) {

    assert(seg.GetStart() < seg.GetEnd());

    MedianCycleSeg s(seg, pFace, parent);

    segs.push_back(s);

    if (firstAppend) {

        minimal1 = s;
        firstAppend = false;
        return;
    }

    if (seg.GetStart() < minimal1.seg.GetStart())
        minimal1 = s;
    else if (seg.GetStart() == minimal1.seg.GetStart())
        minimal2 = s;
}

TypeOfCycle MedianCycle::GetType() const {

    Vector2D first(minimal1.pFace->GetNormalVectorOfResultFace());
    Vector2D second(minimal2.pFace->GetNormalVectorOfResultFace());

    first.Normalize();
    second.Normalize();
    
    Vector2D sum = first + second;

    assert(!NumericUtil::NearlyEqual(sum.GetX(), 0.0));

    if (sum.GetX() > 0.0)
        return HOLE_CYCLE;
    else
        return OUTER_CYCLE;
}

/*

 Class ResultMovingCycle

*/

unsigned int ResultMovingCycle::instanceCount = 0;

/*

 Class ResultCycleContainer

*/

ResultCycleContainer::~ResultCycleContainer() {

    for (unsigned int i = 0; i < faces.size(); i++)
        delete faces[i];

    for (unsigned int i = 0; i < holes.size(); i++)
        delete holes[i];
}

ResultMovingCycle* ResultCycleContainer::GetNewResultCycle() const {

    return ResultMovingCycle::CreateInstance();
}

void ResultCycleContainer::Add(ResultMovingCycle* cycle) {

    assert(cycle->GetType() != UNKNOWN_CYCLE);

    if (cycle->GetType() == OUTER_CYCLE)
        faces.push_back(cycle);
    else
        holes.push_back(cycle);
}

ResultMovingFaces ResultCycleContainer::GetResultMovingFaces() const {

    ResultMovingFaces resultMovingFaces;

    const int noFaces = faces.size();
    const ResultMovingCycle* cycle;

    for (int i = 0; i < noFaces; i++) {

        cycle = faces[i];

        assert(cycle->GetType() == OUTER_CYCLE);

        ResultMovingFace face;
        face.AppendCycle(*cycle);

        const int noHoles = cycle->GetNoHoleCycles();

        for (int h = 0; h < noHoles; h++) {

            face.AppendCycle(*(cycle->GetHoleCycle(h)));
        }

        resultMovingFaces.AppendFace(face);
    }

    return resultMovingFaces;
}

void ResultCycleContainer::LinkHolesToFaces() {

    if (!HasHoles()) {

        // Nothing to link...
        return;
    }

    //cout << "faces: " << faces.size() << endl;
    //cout << "holes: " << holes.size() << endl;
    
    InitSets();
    StartPlaneSweep();
}

void ResultCycleContainer::InitSets() {

    const MedianCycle* mc;

    for (unsigned int i = 0; i < faces.size(); i++) {

        mc = faces[i]->GetMedianCycle();
        for (unsigned int j = 0; j < mc->GetNoSegs(); j++) {

            MedianCycleSeg* seg = mc->GetSeg(j);
            if (!seg->IsVertical()) {
                
                //cout << "fSegs.insert: ";
                //seg->Print();
                //cout << endl;
                
                fSegs.insert(mc->GetSeg(j));
            }
        }
    }
    
    //cout << "fSegs: " << fSegs.size() << endl;

    for (unsigned int i = 0; i < holes.size(); i++) {

        mc = holes[i]->GetMedianCycle();
        
        //cout << "hSegs.insert: ";
        // mc->GetUpperMinimalSeg()->Print();
        //cout << endl;
        
        hSegs.insert(mc->GetUpperMinimalSeg());
    }
    
    //cout << "hSegs: " << hSegs.size() << endl;
}

void ResultCycleContainer::StartPlaneSweep() {

    fSegIter = fSegs.begin();
    hSegsIter = hSegs.begin();

    while (hSegsIter != hSegs.end()) {

        // Get the next relevant x-value:
        x = (*hSegsIter)->seg.GetStart().GetX();
        
        ComputeCurrentXLevel();
        hSegsIter++;
    }
}

void ResultCycleContainer::ComputeCurrentXLevel() {
    
    //cout << "x-level: " << x << endl;

    // remove old segs from active:
    activeIter = active.begin();

    while (activeIter != active.end()) {

        if (IsOutOfRange(*activeIter))
            activeIter = active.erase(activeIter);
        else
            activeIter++;
    }

    // add new segs to active:
    while (fSegIter != fSegs.end() && StartsLeftOfCurrentX(*fSegIter)) {

        if (EndsRightOfCurrentX(*fSegIter))
            active.push_back(*fSegIter);

        fSegIter++;
    }
    
    //cout << "active: " << active.size() << endl;

    FindEnclosingFace();
}

void ResultCycleContainer::FindEnclosingFace() {

    ResultMovingCycle* hole = (*hSegsIter)->cycle;
    ResultMovingCycle* face;

    activeIter = active.begin();
    multiset<MedianCycleSeg*, MedianCycleSegCompareByCycleID> segsAbove;
    multiset<MedianCycleSeg*>::iterator segsAboveIter;

    while (activeIter != active.end()) {
        
        if ((*activeIter)->IsAbove(*hSegsIter))
            segsAbove.insert(*activeIter);

        activeIter++;
    }

    assert(segsAbove.size() > 0);

    segsAboveIter = segsAbove.begin();
    MedianCycleSeg* seg1;
    bool enclosingCycleFound = false;

    while (!enclosingCycleFound) {

        assert(segsAboveIter != segsAbove.end());

        seg1 = *segsAboveIter;
        segsAboveIter++;

        if (segsAboveIter == segsAbove.end() || 
                !(*segsAboveIter)->SameCycle(seg1)) {

            enclosingCycleFound = true;
            face = seg1->cycle;
            face->AddHoleCycle(hole);
        }

        segsAboveIter++;
    }
}

/*

 Struct Mate

*/

void Mate::Print() const {
    
    cout << "StartTime: " << startTime << endl;
    intSeg->Print();
}

/*

 Class MateList

*/

void MateList::AddMate(const double startTime, IntersectionSegment* m) {

    if (mates.empty() || mates.back().intSeg->GetID() != m->GetID()) {

        mates.push_back(Mate(startTime, m));
    }
}

void MateList::Update(const double t) {
    
    assert(!mates.empty());

    list<Mate>::const_iterator iter = mates.begin();
    
    iter++; // 
    
    if (iter != mates.end() &&
        NumericUtil::GreaterOrNearlyEqual(t, (*iter).startTime)) {
        
        mates.pop_front();
    }
}

void MateList::Print() const {

    list<Mate>::const_iterator it;

    for (it = mates.begin(); it != mates.end(); ++it) {

        (*it).Print();
        cout << endl;
    }
}

/*

 Class ResultUnitFactory

*/

void ResultUnitFactory::Start() {
    
    if (source.empty()) {
        
        ResultIsEmpty();
        return;
    }

    sourceIter = source.begin();
    t1 = (*sourceIter)->GetStartT();
    
    UpdateActiveSegList();

    do {

        // Set the final timelevel:
        if (sourceIter != source.end())
            t2 = min((*sourceIter)->GetStartT(), minEndT);
        else
            t2 = minEndT;

        //cout << "t1 = " << t1 << endl;
        //cout << "t2 = " << t2 << endl;
        //cout << "minEndT = " << minEndT << endl;

        ComputeCurrentTimeLevel();
      
        // Set the new initial timelevel:
        t1 = t2;

        UpdateActiveSegList();

    } while (sourceIter != source.end() || !active.empty());
}

void ResultUnitFactory::PrintActive() const {

    if (active.empty())
        cout << "active is empty." << endl;

    for (list<IntersectionSegment*>::const_iterator it = active.begin(); it
            != active.end(); it++) {

        (*it)->Print();
    }
}


void ResultUnitFactory::ComputeCurrentTimeLevel() {
    
    if(active.empty()) {
        
        ResultIsEmpty();
        return;
    }
    
    resultCycleContainer = new ResultCycleContainer();
    const double t12 = (t1 + t2) / 2.0;
    
    activeIter = active.begin();
    
    while (activeIter != active.end()) {

        if (!(*activeIter)->IsInserted()) {

            const IntersectionSegment* firstSegInCycle = *activeIter;
            IntersectionSegment* currentSeg = *activeIter;
            IntersectionSegment* prevSeg = 0;
            pair<IntersectionSegment*, const PFace*> temp;
            
            ResultMovingCycle* cycle = 
                resultCycleContainer->GetNewResultCycle();
            
            //int count = 0;
            //cout << "firstSegInCycle: " << firstSegInCycle->GetID() << endl;
            
            Point2D initialStart(currentSeg->Evaluate(t1));
            Point2D mediumStart(currentSeg->Evaluate(t12));
            Point2D finalStart(currentSeg->Evaluate(t2));

            cycle->StartBulkLoad();
            
            do {
                /*
                cout << currentSeg->GetID() << ": " 
                     << currentSeg->GetStartXYT()->GetX() << " | " 
                     << currentSeg->GetStartXYT()->GetY() << " | " 
                     << currentSeg->GetStartXYT()->GetT() << " -> "
                     << currentSeg->GetEndXYT()->GetX() << " | " 
                     << currentSeg->GetEndXYT()->GetY() << " | " 
                     << currentSeg->GetEndXYT()->GetT() << endl;
                 */
                
                currentSeg->MarkAsInserted();
                
                temp = currentSeg->GetNextActiveMateOfResultCycle(prevSeg);
                
                prevSeg = currentSeg;
                currentSeg = temp.first;
                
                Point2D initialEnd(currentSeg->Evaluate(t1));
                Point2D mediumEnd(currentSeg->Evaluate(t12));
                Point2D finalEnd(currentSeg->Evaluate(t2));
                
                Segment2D initial(initialStart, initialEnd, false);
                Segment2D medium(mediumStart, mediumEnd, true);
                Segment2D final(finalStart, finalEnd, false);
                
                cycle->Append(initial, 
                              medium,
                              final,
                              temp.second);

                initialStart = initialEnd;
                mediumStart = mediumEnd;
                finalStart = finalEnd;
                
                //if (count++ == 20) break;

            } while (currentSeg->GetID() != firstSegInCycle->GetID());
            
            cycle->EndBulkLoad();
            resultCycleContainer->Add(cycle);
        }

        activeIter++;
    }
    
    //cout << "unit: " << t1 << " -> " << t12 << " -> " << t2 << endl;
    
    ConstructResultUnitAsListExpr();
    delete resultCycleContainer;
}

void ResultUnitFactory::UpdateActiveSegList() {

    activeIter = active.begin();
    minEndT = MAX_DOUBLE;

    while (activeIter != active.end()) {

        // Remove segments from active, if out of range:
        if (IsOutOfRange(*activeIter)) {

            activeIter = active.erase(activeIter);

        } else { // segment remains in list.

            UpdateMinEndT();
            (*activeIter)->UpdateMateList(t1);
            (*activeIter)->MarkAsNotInserted();
            
            activeIter++;
        }
    }
    
    // Append new segments from source at the end of active:
    while (HasMoreSegsToInsert()) {

        IntersectionSegment* newSeg = *sourceIter;
        activeIter = active.insert(activeIter, newSeg);
        UpdateMinEndT();
        (*activeIter)->UpdateMateList(t1);
        (*activeIter)->MarkAsNotInserted();
        
        sourceIter++;
        activeIter = active.end();
    }
}

void ResultUnitFactory::ConstructResultUnitAsListExpr() {
    
    resultCycleContainer->LinkHolesToFaces();
    
    ResultUnit unit(t1, t2, true, false);
    unit.AppendFaces(resultCycleContainer->GetResultMovingFaces());
    resultUnitList.AppendUnit(unit);
}

void ResultUnitFactory::ConstructResultUnitAsURegionEmb() {
    
    resultCycleContainer->LinkHolesToFaces();
    
    Instant startTime(instanttype);
    Instant endTime(instanttype);
    startTime.ReadFrom(t1);
    endTime.ReadFrom(t2);
    const Interval<Instant> iv(startTime, endTime, true, false);
    const DBArray<MSegmentData>* resArrayR = resMRegion->GetMSegmentData();
    const unsigned int startPos = resArrayR->Size();
    
    URegionEmb unit(iv, startPos);
    
    DBArray<MSegmentData>* resArrayRW = 
        (DBArray<MSegmentData>*)resMRegion->GetFLOB(1);
    const unsigned int noFaces = resultCycleContainer->GetNoFaces();
    const ResultMovingCycle* face;
    const ResultMovingCycle* hole;
    unsigned int mSegCount = 0;
    
    for (unsigned int i = 0; i < noFaces; i++) {
        
        face = resultCycleContainer->GetFace(i);
        
        // TODO: Construct MSegmentData from face and 
        // add them to unit:
        //MSegmentData mSeg(...);
        //unit.PutSegment(resArrayRW, mSegCount++, mSeg, true);
        
    }
    
    resMRegion->Add(unit);
}

void ResultUnitFactory::ResultIsEmpty() {

    // Nothing to do.
    cout << "ResultUnitFactory::ResultIsEmpty()" << endl;
}

void ResultUnitFactory::ResultIsUnitA() {

    // TODO
    
    assert(false);
}

void ResultUnitFactory::PrintIntSegsOfGlobalList() {

    cout << "*********************************************" << endl;
    cout << "globalIntSegSet:" << endl;
    cout << source.size() << " segments" << endl;
    cout << "*********************************************" << endl;
    cout << endl;

    multiset<IntersectionSegment*>::iterator iter;
    for (iter = source.begin(); 
         iter != source.end(); ++iter) {

        (*iter)->Print();
        cout << endl;
    }
}

void ResultUnitFactory::PrintIntSegsOfGlobalListAsVRML(ofstream& target,
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
    for (iter = source.begin(); 
         iter != source.end(); ++iter) {

        target << "\t\t\t\t\t\t" << (*iter)->GetVRMLDesc() << endl;
    }

    target << "\t\t\t\t\t]  # end point" << endl;
    target << "\t\t\t\t} # end coord" << endl;
    target << "\t\t\t\tcoordIndex [" << endl;

    const int noPoints = source.size() * 2;

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

/*

Class SourceUnit

*/

SourceUnit::SourceUnit(const bool _isUnitA, 
		     const URegionEmb* const _uRegion,
		     const DBArray<MSegmentData>* _array, 
		     SourceUnitPair* const _parent) :
		    	 
	        uRegion(_uRegion), 
	        array(_array),
	        parent(_parent), 
			startTime(_uRegion->timeInterval.start.ToDouble()),
			endTime(_uRegion->timeInterval.end.ToDouble()),
			medianRegion(0),
			isUnitA(_isUnitA) {

	ComputeBoundingRect();
	hasMedianRegion = false;
	medianTime = GetTimeInterval().start + 
	 ((GetTimeInterval().end - GetTimeInterval().start) * 0.5);
	
	//cout << "medianTime = " << medianTime.ToDouble() << endl;
}

void SourceUnit::ComputeBoundingRect() {

	// Calculate the projection bounding rectangle in the (x, y)-plane
	// of the URegion:
	
	double minX = uRegion->BoundingBox().MinD(0);
	double maxX = uRegion->BoundingBox().MaxD(0);
	double minY = uRegion->BoundingBox().MinD(1);
	double maxY = uRegion->BoundingBox().MaxD(1);
	
	boundingRect = Rectangle<2>(true, minX, maxX, minY, maxY);
}

void SourceUnit::ComputeMedianRegion() {
	
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

void SourceUnit::CreatePFaces() {

    const MSegmentData* segment;

    int cycleNo = -1;
    int faceNo = -1;
    cycleCount = 0;
    pFaceCountTotal = 0;

    for (int i = 0; i < uRegion->GetSegmentsNum(); i++) {

        uRegion->GetSegment(array, i, segment);

        if ((int)segment->GetCycleNo() != cycleNo ||
            (int)segment->GetFaceNo() != faceNo) {

            // We encountered a new cycle:
            // Add the new cycle:
            pFacesCycles.push_back(PFaceCycle());
            cycleCount++;
            cycleNo = segment->GetCycleNo();
            faceNo = segment->GetFaceNo();
        }

        //cout << endl;
        //cout << endl;
        //cout << "segment->GetSegmentNo() = " 
        //<< segment->GetSegmentNo() << endl;
        //cout << "segment->GetInsideAbove() = " 
        //<< segment->GetInsideAbove() << endl;
        //cout << endl;
        //cout << segment->ToString() << endl;
        //cout << endl;

        PFace* pFace = new PFace(this, segment);

        assert(!pFacesCycles.empty());

        pFacesCycles.back().Add(pFace);
        pFaceCountTotal++;

#ifdef REDUCE_PFACES_BY_BOUNDING_RECT

        if (pFace->GetBoundingRect().Intersects(parent->GetOverlapRect()))
            pFacesReduced.push_back(pFace);
        else
            pFace->MarkAsEntirelyOutside();

#else
        pFacesReduced.push_back(pFace);
#endif

        //pFace->Print();
    }
    
    ConnectPFacesInCycles();

    //cout << "cycleCount = " << cycleCount << endl;
    //cout << "pFaceCount = " << pFaceCountTotal << endl;
    //cout << "pFacesReducedCount = " << pFacesReduced.size() << endl;
}

void SourceUnit::ConnectPFacesInCycles() {
    
    list<PFaceCycle>::iterator iter;
    
    for (iter = pFacesCycles.begin(); iter != pFacesCycles.end(); iter++)
        (*iter).ConnectPFaces();
}

void SourceUnit::PrintPFaceCycles() {

	int cycleCount = 0;

	for (list<PFaceCycle>::iterator iter = pFacesCycles.begin(); iter
			!= pFacesCycles.end(); iter++) {
		
		cout << "*********************************************" << endl;
		cout << "Cycle No: " << cycleCount << endl;
		cout << "*********************************************" << endl;

		PFaceCycleIterator pIter = (*iter).GetPFaceCycleIterator();

		while (pIter.HasNext()) {

			pIter.GetNext()->Print();
		}

		cout << endl;
		cycleCount++;
	}
}

void SourceUnit::PrintPFacePairs() {

    int cycleCount = 0;

    for (list<PFaceCycle>::iterator iter = pFacesCycles.begin(); iter
            != pFacesCycles.end(); iter++) {

        cout << "*********************************************" << endl;
        cout << "Cycle No: " << cycleCount << endl;
        cout << "*********************************************" << endl;

        PFaceCycleIterator pIter = (*iter).GetPFaceCycleIterator();

        int pairCount = 0;

        while (pIter.HasNextPair()) {

            PFacePair pair = pIter.GetNextPair();

            cout << "Pair No: " << pairCount << endl;
            cout << endl;
            cout << "Left PFace: " << endl;
            cout << endl;
            pair.GetLeft()->Print();
            cout << endl;
            cout << "*********************************************" << endl;
            cout << endl;
            cout << "Right PFace: " << endl;
            cout << endl;
            pair.GetRight()->Print();
            cout << endl;
            cout << "*********************************************" << endl;
            cout << "*********************************************" << endl;

            pairCount++;
        }

        cout << endl;
        cycleCount++;
    }
}

void SourceUnit::PrintIntSegsOfPFaces() {

    int cycleNo = 0;

    for (list<PFaceCycle>::iterator iter = pFacesCycles.begin(); iter
            != pFacesCycles.end(); iter++) {

        cout << "*********************************************" << endl;
        cout << "Cycle No: " << cycleNo << endl;
        cout << "*********************************************" << endl;

        int pFaceNo = 0;

        PFaceCycleIterator pIter = (*iter).GetPFaceCycleIterator();

        while (pIter.HasNext()) {

            cout << "*********************************************" << endl;
            cout << "PFace No: " << pFaceNo << endl;
            cout << "*********************************************" << endl;

            PFace* pf = pIter.GetNext();
            pf->PrintGeneralIntSegs();
            pf->PrintRightIntSegs();
            pf->PrintOrthogonalIntSegs();

            pFaceNo++;
        }

        cout << endl;
        cycleNo++;
    }
}

void SourceUnit::PrintVRMLDesc(ofstream& target, const string& color) {

    const double scale= VRML_SCALE_FACTOR;

    target << "Transform {" << endl;
    target << "\tscale " << scale << " " << scale << " " << scale << endl;
    target << "\tchildren [" << endl;
    target << "\t\tShape {" << endl;
    target << "\t\t\tgeometry IndexedLineSet {" << endl;
    target << "\t\t\t\tcoord Coordinate {" << endl;
    target << "\t\t\t\t\tpoint [" << endl;

    int pFaceCount = 0;

    for (list<PFaceCycle>::iterator iter = pFacesCycles.begin(); iter
            != pFacesCycles.end(); iter++) {

        PFaceCycleIterator pIter = (*iter).GetPFaceCycleIterator();

        while (pIter.HasNext()) {

            target << "\t\t\t\t\t\t" << pIter.GetNext()->GetVRMLDesc() << endl;
            pFaceCount++;
        }
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

void SourceUnit::AddBoundarySegments() {

    bool firstPairInCycle;
    const SetOp op = this->GetOperation();

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
                
                if (!pair.GetLeft()->HasIntSegsColinearToRightBoundary()
                    && !pair.GetRight()->HasIntSegsColinearToLeftBoundary()) {

                    pair.ComputeBoundarySegmentsNormal();
                    
                } else {
                    cout << "pair.ComputeBoundarySegmentsSpecial() called." 
                         << endl;
                    pair.ComputeBoundarySegmentsSpecial();
                }
            }

            firstPairInCycle = false;
        }
    }
}

void SourceUnit::FindMates() {

    for (list<PFaceCycle>::iterator iter = pFacesCycles.begin(); iter
            != pFacesCycles.end(); iter++) {

        PFaceCycleIterator pIter = (*iter).GetPFaceCycleIterator();

        while (pIter.HasNext()) {

            pIter.GetNext()->FindMates();
        }
    }
}

void SourceUnit::CollectIntSegs(ResultUnitFactory* receiver) {

    for (list<PFaceCycle>::iterator iter = pFacesCycles.begin(); iter
            != pFacesCycles.end(); iter++) {

        PFaceCycleIterator pIter = (*iter).GetPFaceCycleIterator();

        while (pIter.HasNext()) {

            pIter.GetNext()->CollectIntSegs(receiver);
        }
    }
}

void SourceUnit::FindMatesAndCollectIntSegs(ResultUnitFactory* receiver) {

    for (list<PFaceCycle>::iterator iter = pFacesCycles.begin(); iter
            != pFacesCycles.end(); iter++) {

        PFaceCycleIterator pIter = (*iter).GetPFaceCycleIterator();

        while (pIter.HasNext()) {

            PFace* pFace = pIter.GetNext();
            pFace->FindMates();
            pFace->CollectIntSegs(receiver);
        }
    }
}

bool SourceUnit::IsEntirelyOutside(const PFace* pFace) {

    // Check, if PFace's location is already known:

    if (pFace->IsEntirelyOutside())
        return true;

    if (pFace->IsEntirelyInside())
        return false;

    // We don't know and have to use the plumbline algorithm
    // to find the answer:

    //cout << "Plumbline used." << endl;

    //Vector3D aToMiddle = (pFace->GetC_XYT() - pFace->GetA_XYT()) * 0.5;
    Point3D middleXYT = (pFace->GetA_XYT() + pFace->GetC_XYT()) * 0.5;
    Point middleXY(middleXYT.GetX(), middleXYT.GetY());

    //cout << "middleXYT: " << middleXYT.GetX() << " | " << middleXYT.GetY()
            //<< " | " << middleXYT.GetT() << endl;

    return !GetMedianRegion().Contains(middleXY);
}

const SetOp SourceUnit::GetOperation() const {

    return parent->GetOperation();
}

/*

 Class SourceUnitPair

*/

SourceUnitPair::SourceUnitPair(const URegionEmb* const _unitA,
                               const DBArray<MSegmentData>* const _aArray, 
                               const URegionEmb* const _unitB,
                               const DBArray<MSegmentData>* const _bArray,
                               const SetOp _operation,
                               //MRegion* const resultMRegion,
                               NList* const _resultUnitList) :

    unitA(true, _unitA, _aArray, this), 
    unitB(false, _unitB, _bArray, this),
    op(_operation),
    //resultUnitList(_resultUnitList),
    resultUnitFactory(//resultMRegion, 
                      _resultUnitList,
                      this) {
    
    assert(NumericUtil::NearlyEqual(_unitA->timeInterval.start.ToDouble(), 
                                    _unitB->timeInterval.start.ToDouble()) 
            
       &&  NumericUtil::NearlyEqual(_unitA->timeInterval.end.ToDouble(), 
                                    _unitB->timeInterval.end.ToDouble()));  

    unitA.SetPartner(&unitB);
    unitB.SetPartner(&unitA);

    ComputeOverlapRect();
}

void SourceUnitPair::Operate() {

    //if (op == UNION || HasOverlappingBoundingRect()) {
    if (true) {

        CreatePFaces();
        //return;
        //PrintPFaceCycles();
        //return;
        //PrintPFacePairs();
        ComputeInnerIntSegs();
        //PrintIntSegsOfPFaces();
        AddBoundarySegments();
        //PrintIntSegsOfPFaces();
        //FindMates();
        //PrintIntSegsOfPFaces();
        FindMatesAndCollectIntSegs(&resultUnitFactory);
        //CollectIntSegs(&resultUnitFactory);
        //PrintIntSegsOfPFaces();
        //resultUnitFactory.PrintIntSegsOfGlobalList();
        //PrintIntSegsOfGlobalListAsVRML();
        PrintUnitsAsVRML();
        ConstructResultUnits();

    } else {

        switch (op) {
        
        case MINUS:
                    
            resultUnitFactory.ResultIsUnitA();
            break;

        case INTERSECTION:

            resultUnitFactory.ResultIsEmpty();
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

void SourceUnitPair::ComputeInnerIntSegs() {

    for (list<PFace*>::iterator iterA = unitA.pFacesReduced.begin(); iterA
            != unitA.pFacesReduced.end(); iterA++) {

        for (list<PFace*>::iterator iterB = unitB.pFacesReduced.begin(); iterB
                != unitB.pFacesReduced.end(); iterB++) {

            (*iterA)->Intersection(**iterB);
        }
    }
}

void SourceUnitPair::AddBoundarySegments() {

    unitA.AddBoundarySegments();
    unitB.AddBoundarySegments();
}

void SourceUnitPair::FindMates() {

    unitA.FindMates();
    unitB.FindMates();
}

void SourceUnitPair::CollectIntSegs(ResultUnitFactory* receiver) {

    unitA.CollectIntSegs(receiver);
    unitB.CollectIntSegs(receiver);
}

void SourceUnitPair::FindMatesAndCollectIntSegs(ResultUnitFactory* receiver) {

    unitA.FindMatesAndCollectIntSegs(receiver);
    unitB.FindMatesAndCollectIntSegs(receiver);
}

void SourceUnitPair::ConstructResultUnits() {
    
        resultUnitFactory.Start();
}

void SourceUnitPair::PrintPFaceCycles() {

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

void SourceUnitPair::PrintPFacePairs() {

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

void SourceUnitPair::PrintIntSegsOfPFaces() {

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



void SourceUnitPair::PrintUnitsAsVRML() {

    const string colorA = "1 1 0";
    const string colorB = "0 0.6 1";
    const string colorIntSegs = "1 0 0";

    const string filename = "unit_" + 
                            GetTimeInterval().start.ToString() + 
                            ".wrl";
    
    ofstream target(filename.c_str());

    if (!target.is_open()) {

        cerr << "Unable to open file: " << filename << endl;
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
    resultUnitFactory.PrintIntSegsOfGlobalListAsVRML(target, colorIntSegs);

    target.close();
}

/*

 Class SetOperator

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
    
    // Note: Only one unit is supported!

    if (!a->IsDefined() || !b->IsDefined()) {

        res->SetDefined(false);
        return;
    }

    const URegionEmb* unitA;
    const URegionEmb* unitB;
    const DBArray<MSegmentData>* aArray;
    const DBArray<MSegmentData>* bArray;
    
    a->Get(0, unitA);
    b->Get(0, unitB);
    
    aArray = a->GetMSegmentData();
    bArray = b->GetMSegmentData();

    SourceUnitPair so = SourceUnitPair(unitA, aArray, 
                                       unitB, bArray, 
                                       op, 
                                       &resultMRegionList);
    so.Operate();
    
    MRegion* operationResult = CreateResultMRegionFromList();
    res->CopyFrom(operationResult);
    delete operationResult;
}



void SetOperator::AppendUnitList(const NList* resultUnitList) {
    
    resultMRegionList.append(*resultUnitList);
}

MRegion* SetOperator::CreateResultMRegionFromList() {
    
    //cout << resultMRegionList << endl;
    
    ListExpr errorInfo = NList().listExpr();
    bool correct = false;
        
    Word result = InMRegion(NList("movingregion").listExpr(),
                            resultMRegionList.listExpr(),
                            0,
                            errorInfo,
                            correct);
    
    if (correct) {
    
        return (MRegion*)result.addr;
        
    } else {
        
        cerr << "resultMRegionListExpr is incorrect:" << endl;
        cerr << resultMRegionList << endl;
        cerr << NList(errorInfo) << endl;
        
        MRegion* result = new MRegion(0);
        result->SetDefined(false);
        
        return result;
    }
}


bool GeneralIntSegSetCompare::operator()(const IntersectionSegment* const& s1,
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
    return s1->GetEndWT().IsLeft(s2->GetStartWT(), s2->GetEndWT());
}

bool RightIntSegSetCompare::operator()(const IntersectionSegment* const& s1,
        const IntersectionSegment* const& s2) const {

    double t1, t2, t3, t4;

    if (s1->GetRelationToBoundaryOfPFace() == 
        TOUCH_RIGHT_BOUNDARY_IN_STARTPOINT_ONLY) {

        t1 = s1->GetStartT();
        t2 = s1->GetEndT();

    } else { // TOUCH_IN_ENDPOINT

        t1 = s1->GetEndT();
        t2 = s1->GetStartT();
    }

    if (s2->GetRelationToBoundaryOfPFace() == 
        TOUCH_RIGHT_BOUNDARY_IN_STARTPOINT_ONLY) {

        t3 = s2->GetStartT();
        t4 = s2->GetEndT();

    } else { // TOUCH_IN_ENDPOINT

        t3 = s2->GetEndT();
        t4 = s2->GetStartT();
    }

    if (NumericUtil::NearlyEqual(t1, t3))
        return NumericUtil::Lower(t2, t4);
    
    return NumericUtil::Lower(t1, t3);
}

bool ColinearIntSegSetCompare::operator()(const IntersectionSegment* const& s1,
        const IntersectionSegment* const& s2) const {
    
    return NumericUtil::Lower(s1->GetStartT(), s2->GetStartT());
}

/*

 Class MateEngine

*/


void MateEngine::ComputeCurrentTimeLevel() {

    activeIter = active.begin();
    minEndT = MAX_DOUBLE;

    while (activeIter != active.end()) {

        while (activeIter != active.end() && IsOutOfRange(*activeIter)) {

            activeIter = active.erase(activeIter);
        }

        if (activeIter == active.end())
            break;

        if (HasMoreSegsToInsert()) {

            IntersectionSegment* newSeg = *sourceIter;

            if (newSeg->IsLeftOf(*activeIter)) {

                activeIter = active.insert(activeIter, newSeg);
                sourceIter++;
            }
        }

        DoMating();
        UpdateMinEndT();
        activeIter++;
    }

    assert(activeIter == active.end());

    // Add the tail, if there is one:
    while (HasMoreSegsToInsert()) {

        IntersectionSegment* newSeg = *sourceIter;
        activeIter = active.insert(activeIter, newSeg);
        DoMating();
        UpdateMinEndT();
        sourceIter++;
        activeIter = active.end();
    }
}

void MateEngine::DoMating() {

    IntersectionSegment* current = *activeIter;
    IntersectionSegment* pred;
    bool hasPred = false;

    if (activeIter != active.begin()) {

        activeIter--;
        pred = *activeIter;
        hasPred = true;
        activeIter++;
    }

    if (!hasPred || pred->IsRightBoundary()) {

        current->MarkAsLeftBoundary();

    } else { // pred->IsLeftBoundary() == true

        current->MarkAsRightBoundary();
        current->AddMate(t, pred);
        pred->AddMate(t, current);
    }
}

/*

 Class PFace

*/

PFace::PFace(SourceUnit* _unit, const MSegmentData* _mSeg) :

    unit(_unit), mSeg(_mSeg) {

    hasIntSegs = false;
    hasBoundaryIntSegs = false;
    entirelyInsideOutside = UNKNOWN;
    rightNeighbour = 0;
    leftNeighbour = 0;

    SetInitialStartPointIsA();
    ComputeBoundingRect();
    ComputeNormalVector();
    ComputeWTCoords();
}

void PFace::SetInitialStartPointIsA() {
    
    double startX, startY, endX, endY;
    
    if (!mSeg->GetPointInitial()) {
        
        startX = mSeg->GetInitialStartX();
        startY = mSeg->GetInitialStartY();
        endX = mSeg->GetInitialEndX();
        endY = mSeg->GetInitialEndY();
        
    } else {
        
        startX = mSeg->GetFinalStartX();
        startY = mSeg->GetFinalStartY();
        endX = mSeg->GetFinalEndX();
        endY = mSeg->GetFinalEndY();
    }
    
    if (NumericUtil::Lower(startX, endX))
        initialStartPointIsA = mSeg->GetInsideAbove();
    else
        if (NumericUtil::Greater(startX, endX))
            initialStartPointIsA = !mSeg->GetInsideAbove();
        else // startX equals endX
            if (NumericUtil::Lower(startY, endY))
                initialStartPointIsA = mSeg->GetInsideAbove();
            else
                initialStartPointIsA = !mSeg->GetInsideAbove();
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
        return false;

    // Compute segment intersection point:
    result = seg.GetStart() + s * u;

    return true;
}

bool PFace::Intersection(PFace& other) {
    
    //if (!this->GetBoundingRect().Intersects(other.GetBoundingRect()))
        //return false;

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
    
    // We got an intersection segment buddy pair:
    result.first->SetPFace(this);
    result.second->SetPFace(&other);
    
    // and compute the w-coords of each segment:
    result.first->SetWCoords();
    result.second->SetWCoords();

    // Check, on which side of the segment the result face is:
    result.first->SetSideOfResultFace(*this, other);
    result.second->SetSideOfResultFace(other, *this);
    
    // Determine the relation of the segment to the PFace's boundary:
    result.first->SetRelationToBoundaryOfPFace();
    result.second->SetRelationToBoundaryOfPFace();
    
    
    if (result.first->ResultFaceIsVanished() ||
        result.second->ResultFaceIsVanished() ||
        result.first->IsTouchingLine(other) ||
        result.second->IsTouchingLine(*this)) {
        
        result.first->Delete();
        result.second->Delete();

        return false;
    }
    
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
    for (iter = generalIntSegSet.begin(); 
         iter != generalIntSegSet.end(); ++iter)
        (*iter)->Delete();
}

void PFace::FindMates() {

    MateEngine mateEngine(&generalIntSegSet);
    mateEngine.Start();
}

void PFace::CollectIntSegs(ResultUnitFactory* receiver) const {

    set<IntersectionSegment*>::iterator iter;
    for (iter = generalIntSegSet.begin(); 
         iter != generalIntSegSet.end(); ++iter)
        receiver->AddIntSeg(*iter);
}

bool PFace::IsParallelTo(const PFace& pf) const {

    Vector3D cross = GetNormalVector() ^ pf.GetNormalVector();

    return NumericUtil::NearlyEqual(cross.GetX(), 0.0)
            && NumericUtil::NearlyEqual(cross.GetY(), 0.0)
            && NumericUtil::NearlyEqual(cross.GetT(), 0.0);
}

void PFace::AddInnerIntSeg(IntersectionSegment* intSeg) {

    const RelationToBoundary relToBoundary = 
        intSeg->GetRelationToBoundaryOfPFace();
    
    assert(relToBoundary != UNDEFINED_RELATION);
    
    if (relToBoundary != NO_TOUCH_OF_BOUNDARY) {
        
        if (relToBoundary == TOUCH_RIGHT_BOUNDARY_IN_STARTPOINT_ONLY ||
            relToBoundary == TOUCH_RIGHT_BOUNDARY_IN_ENDPOINT_ONLY)
            rightIntSegSet.insert(intSeg);
        else
            if (relToBoundary == COLINEAR_TO_RIGHT_BOUNDARY) 
                rightColinearIntSegSet.insert(intSeg); 
            else 
                if (relToBoundary == COLINEAR_TO_LEFT_BOUNDARY) 
                    leftColinearIntSegSet.insert(intSeg);
    }
         
    if (!intSeg->IsOrthogonalToTAxis()) {

        generalIntSegSet.insert(intSeg);

    } else {

        orthogonalIntSegList.push_back(intSeg);
    }

    //relevantTimeValueSet.insert(intSeg->GetStartT());
    //relevantTimeValueSet.insert(intSeg->GetEndT());

    hasIntSegs = true;
    hasInnerIntSegs = true;
}

void PFace::AddBoundaryIntSegs(IntersectionSegment* intSeg) {
    
    intSeg->SetPFace(this);

    // Set the w-coords of the intersection segment, 
    // depending of this PFace:
    intSeg->SetStartW(this->TransformToW(*intSeg->GetStartXYT()));
    intSeg->SetEndW(this->TransformToW(*intSeg->GetEndXYT()));

    generalIntSegSet.insert(intSeg);
    //relevantTimeValueSet.insert(intSeg->GetStartT());
    //relevantTimeValueSet.insert(intSeg->GetEndT());

    hasIntSegs = true;
    hasBoundaryIntSegs = true;
}

const IntersectionSegment* PFace::GetRightmostIntSeg() {

    assert(!generalIntSegSet.empty());

    const IntersectionSegment* rightmost;
    double t;

    set<IntersectionSegment*>::iterator iter;
    iter = generalIntSegSet.end();
    iter--;
    rightmost = *iter;
    t = (*iter)->GetStartT();

    while (true) {

        while (NumericUtil::NearlyEqual((*iter)->GetStartT(), t)) {
            
            // We walk to the last intSeg of the next lower timelevel,
            // if there is one:
            if (iter == generalIntSegSet.begin()) {
                
                //cout << "rightmost intSeg: ";
                //rightmost->Print();
                //cout << endl;
                
                return rightmost;
            }
            
            iter--;
        }
        
        // Update t:
        t = (*iter)->GetStartT();
        
        // Now, iter points to the last intSeg of the current timelevel,
        // which is the rightmost intSeg of this timelevel.
        
        if (rightmost->IsLeftOf(*iter)) {
            
            // This intSeg is right of rightMost:
            rightmost = *iter;
        }
    }

    //return rightmost;
}

string PFace::GetVRMLDesc() {

    return GetA_XYT().GetVRMLDesc() + 
           GetC_XYT().GetVRMLDesc() + 
           GetD_XYT().GetVRMLDesc() + 
           GetB_XYT().GetVRMLDesc();
}

void PFace::Print() const {

    //cout << "PFace No: " << GetNoInCycle() << endl;
    //cout << endl;
    //cout << "Starttime: " << unit->GetStartTime() << endl;
    //cout << "Endtime: " << unit->GetEndTime() << endl;
    cout << "A_XY: " << GetA_XYT().GetX() << " | " << GetA_XYT().GetY() << endl;
    cout << "A_W: " << GetA_WT().GetW() << endl;
    cout << "B_XY: " << GetB_XYT().GetX() << " | " << GetB_XYT().GetY() << endl;
    cout << "B_W: " << GetB_WT().GetW() << endl;
    cout << "C_XY: " << GetC_XYT().GetX() << " | " << GetC_XYT().GetY() << endl;
    cout << "C_W: " << GetC_WT().GetW() << endl;
    cout << "D_XY: " << GetD_XYT().GetX() << " | " << GetD_XYT().GetY() << endl;
    cout << "D_W: " << GetD_WT().GetW() << endl;

    cout << "Normal vector: " << GetNormalVector().GetX() << " | " 
                              << GetNormalVector().GetY() << " | " 
                              << GetNormalVector().GetT() << endl;

    cout << "W vector: " << GetWVector().GetX() << " | " 
                         << GetWVector().GetY() << " | " 
                         << GetWVector().GetT() << endl;

    //cout << "mSeg->GetInsideAbove(): " << mSeg->GetInsideAbove() << endl;

    cout << endl;
    cout << "*********************************************" << endl;
    cout << endl;
}

void PFace::PrintGeneralIntSegs() {

    cout << endl;
    cout << "*********************************************" << endl;
    cout << "generalIntSegSet:" << endl;
    cout << "*********************************************" << endl;
    cout << endl;

    if (generalIntSegSet.empty()) {

        cout << "generalIntSegSet is empty." << endl;
        cout << endl;
        cout << "*********************************************" << endl;
        return;
    }

    set<IntersectionSegment*>::iterator iter;

    for (iter = generalIntSegSet.begin(); 
         iter != generalIntSegSet.end(); ++iter) {

        (*iter)->Print();
        cout << endl;
        cout << "*********************************************" << endl;
        cout << endl;

        cout << "Mates: " << endl;
        cout << "*********************************************" << endl;
        (*iter)->PrintMates();
        cout << "*********************************************" << endl;
        cout << "End of Mates." << endl;
        cout << "*********************************************" << endl;
    }

}

void PFace::PrintRightIntSegs() {

    cout << endl;
    cout << "*********************************************" << endl;
    cout << "rightIntSegSet:" << endl;
    cout << "*********************************************" << endl;
    cout << endl;

    if (rightIntSegSet.empty()) {

        cout << "rightIntSegSet is empty." << endl;
        cout << endl;
        cout << "*********************************************" << endl;
        return;
    }

    set<IntersectionSegment*>::iterator iter;

    for (iter = rightIntSegSet.begin(); iter != rightIntSegSet.end(); ++iter) {

        (*iter)->Print();
        cout << endl;
        cout << "*********************************************" << endl;
        cout << endl;
    }

    cout << "*********************************************" << endl;
}

void PFace::PrintOrthogonalIntSegs() {

    cout << endl;
    cout << "*********************************************" << endl;
    cout << "orthogonalIntSegList:" << endl;
    cout << "*********************************************" << endl;
    cout << endl;

    if (orthogonalIntSegList.empty()) {

        cout << "orthogonalIntSegList is empty." << endl;
        cout << endl;
        cout << "*********************************************" << endl;
        return;
    }

    list<IntersectionSegment*>::iterator iter;

    for (iter = orthogonalIntSegList.begin(); iter
            != orthogonalIntSegList.end(); ++iter) {

        (*iter)->Print();
        cout << endl;
        cout << "*********************************************" << endl;
        cout << endl;
    }

    cout << "*********************************************" << endl;
}

void PFacePair::ComputeBoundarySegmentsNormal() {

    // Precondition: left->HasIntSegs() == true.
    assert(left->HasIntSegs());

    if (!left->HasIntSegsTouchingRightBoundary()) {

        const IntersectionSegment* rightmost = left->GetRightmostIntSeg();

        if (rightmost->ResultFaceIsRight()) {

            // Both, the entire right boundary of PFace left and 
            // the entire left boundary of PFace right 
            // contributes to a result face:

            AddEntireBoundary();

        } else { // intSegMaxW->ResultFaceIsLeft() == true

            // Neither the right boundary of PFace left nor 
            // the left boundary of PFace right 
            // contributes to a result face:
            // do nothing...
        }

    } else { // left->HasIntSegsTouchingRightBoundary() == true

        const Point3D p = left->GetB_XYT();
        const Point3D* p1 = &p;
        const Point3D* p2;
        bool boundaryIntSegFound;

        set<IntersectionSegment*>::iterator iter;

        for (iter = left->rightIntSegSet.begin(); iter
                != left->rightIntSegSet.end(); ++iter) {

            if ((*iter)->GetRelationToBoundaryOfPFace()
                    == TOUCH_RIGHT_BOUNDARY_IN_STARTPOINT_ONLY) {

                p2 = (*iter)->GetStartXYT();
                boundaryIntSegFound = (*iter)->ResultFaceIsLeft();

            } else { // TOUCH_IN_ENDPOINT

                p2 = (*iter)->GetEndXYT();
                boundaryIntSegFound = (*iter)->ResultFaceIsRight();
            }

            if (boundaryIntSegFound) {

                AddBoundarySegment(*p1, *p2);
            }

            p1 = p2;
        }

        // Create the last boundary part:
        if (!boundaryIntSegFound) {

            AddBoundarySegment(*p1, left->GetD_XYT());
        }
    }
}

void PFacePair::ComputeBoundarySegmentsSpecial() {

    assert(left->HasIntSegs());
    
    Range<Instant> boundary(0);
    Instant start(instanttype);
    Instant end(instanttype);

    if (!left->HasIntSegsTouchingRightBoundary()) {

        const IntersectionSegment* rightmost = left->GetRightmostIntSeg();

        if (rightmost->ResultFaceIsRight()) {

            // Both, the entire right boundary of PFace left and 
            // the entire left boundary of PFace right 
            // contributes to a result face:

            //AddEntireBoundary();
            start.ReadFrom(left->GetA_WT().GetT());
            end.ReadFrom(left->GetC_WT().GetT());
            boundary.StartBulkLoad();
            boundary.Add(Interval<Instant>(start, end, true, true));
            boundary.EndBulkLoad(false);

        } else { // intSegMaxW->ResultFaceIsLeft() == true

            // Neither the right boundary of PFace left nor 
            // the left boundary of PFace right 
            // contributes to a result face:
            // do nothing...
        }

    } else { // left->HasIntSegsTouchingRightBoundary() == true

        const Point3D p = left->GetB_XYT();
        const Point3D* p1 = &p;
        const Point3D* p2;
        bool boundaryIntSegFound;
        
        set<IntersectionSegment*>::iterator iter;
        
        boundary.StartBulkLoad();

        for (iter = left->rightIntSegSet.begin(); iter
                != left->rightIntSegSet.end(); ++iter) {

            if ((*iter)->GetRelationToBoundaryOfPFace()
                    == TOUCH_RIGHT_BOUNDARY_IN_STARTPOINT_ONLY) {

                p2 = (*iter)->GetStartXYT();
                boundaryIntSegFound = (*iter)->ResultFaceIsLeft();

            } else { // TOUCH_IN_ENDPOINT

                p2 = (*iter)->GetEndXYT();
                boundaryIntSegFound = (*iter)->ResultFaceIsRight();
            }

            if (boundaryIntSegFound) {

                //AddBoundarySegment(*p1, *p2);
                start.ReadFrom(p1->GetT());
                end.ReadFrom(p2->GetT());
                boundary.Add(Interval<Instant>(start, end, true, true));
            }

            p1 = p2;
        }

        // Create the last boundary part:
        if (!boundaryIntSegFound) {

            //AddBoundarySegment(*p1, left->GetD_XYT());
            start.ReadFrom(p1->GetT());
            end.ReadFrom(left->GetD_XYT().GetT());
            boundary.Add(Interval<Instant>(start, end, true, true));
        }
        
        boundary.EndBulkLoad(false);
    }
    
    Range<Instant> colinearSegsOfLeft(0);
    Range<Instant> colinearSegsOfRight(0);
    colinearSegsOfLeft.StartBulkLoad();
    colinearSegsOfRight.StartBulkLoad();
    
    set<IntersectionSegment*, ColinearIntSegSetCompare>::const_iterator iter;
    
    for (iter = left->rightColinearIntSegSet.begin();
            iter != left->rightColinearIntSegSet.end(); iter++) {

        start.ReadFrom((*iter)->GetStartT());
        end.ReadFrom((*iter)->GetEndT());
        colinearSegsOfLeft.Add(Interval<Instant>(start, end, true, true));
    }
    
    for (iter = right->leftColinearIntSegSet.begin();
            iter != right->leftColinearIntSegSet.end(); iter++) {

        start.ReadFrom((*iter)->GetStartT());
        end.ReadFrom((*iter)->GetEndT());
        colinearSegsOfRight.Add(Interval<Instant>(start, end, true, true));
    }
    
    colinearSegsOfLeft.EndBulkLoad(false);
    colinearSegsOfRight.EndBulkLoad(false);
    
    Range<Instant> colinear(colinearSegsOfLeft.GetNoComponents() + 
                            colinearSegsOfRight.GetNoComponents());
    
    colinearSegsOfLeft.Union(colinearSegsOfRight, colinear);
    
    Range<Instant> resultBoundary(0);
    boundary.Minus(colinear, resultBoundary);
    
    const Vector3D u = left->GetB_XYT() - left->GetD_XYT();
    const double d = left->GetEndTime() - left->GetStartTime();
    
    for (int i = 0; i < resultBoundary.GetNoComponents(); i++) {
        
        const Interval<Instant>* iv;
        resultBoundary.Get(i, iv);
        double t1 = iv->start.ToDouble();
        double t2 = iv->end.ToDouble();
        double n1 = t1 - left->GetStartTime();
        double n2 = t2 - left->GetStartTime();
        
        double s1 = n1 / d;
        double s2 = n2 / d;
        
        assert(NumericUtil::Between(0.0, s1, 1.0));
        assert(NumericUtil::Between(0.0, s2, 1.0));
        
        Point3D p1(left->GetB_XYT() + s1 * u);
        Point3D p2(left->GetB_XYT() + s2 * u);
        
        AddBoundarySegment(p1, p2);
    }
}

/*

Class PFaceCycle

*/

PFaceCycleIterator PFaceCycle::GetPFaceCycleIterator() {

	return PFaceCycleIterator(this);
}

void PFaceCycle::ConnectPFaces() {

    PFaceCycleIterator iter = GetPFaceCycleIterator();

    while (iter.HasNextPair()) {

        PFacePair pair = iter.GetNextPair();
        pair.GetLeft()->SetRightNeighbour(pair.GetRight());
        pair.GetRight()->SetLeftNeighbour(pair.GetLeft());
    }
}

} // end of namespace mregionops
