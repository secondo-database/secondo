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

