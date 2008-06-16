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

bool PFace::IntersectsRightBoundary(IntersectionSegment* intSeg) const {

    if (intSeg->GetRelationToRightBoundaryOfPFace() == NO_TOUCH)
        return false;

    const double s = intSeg->GetStartWT().IsLeftValue(this->GetB_WT(), 
            this->GetD_WT());
    const bool startOnBoundary = NumericUtil::NearlyEqual(s, 0.0);

    const double e = intSeg->GetEndWT().IsLeftValue(this->GetB_WT(), 
            this->GetD_WT());
    const bool endOnBoundary = NumericUtil::NearlyEqual(e, 0.0);

    if (startOnBoundary && !endOnBoundary) {

        intSeg->SetRelationToRightBoundaryOfPFace(TOUCH_IN_STARTPOINT);
        return true;
    }

    if (!startOnBoundary && endOnBoundary) {

        intSeg->SetRelationToRightBoundaryOfPFace(TOUCH_IN_ENDPOINT);
        return true;
    }

    intSeg->SetRelationToRightBoundaryOfPFace(NO_TOUCH);
    return false;
}
