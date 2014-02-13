/*
  1 Class MFace represents a Moving Face with optional holes

*/

#include "interpolate.h"

MFace::MFace() : needStartRegion(false), needEndRegion(false) {
}

/*
 1.1 Constructs a Moving Face from a set of Moving Segments
 
*/
MFace::MFace(MSegs face) : face(face),
        needStartRegion(false), needEndRegion(false)
{
    Check();
}

/*
 1.2 SortCycle

 Sort the Moving Segments according to their position in the cycle and do
 several sanity-checks.
 
*/
bool MFace::SortCycle() {
    vector<MSeg> s1 = face.msegs;
    vector<MSeg> s2;

    int cur = 0;
    unsigned int i = 0;
    bool ret = true;
    
    //Start with the first segment
    do {
        s2.push_back(s1[cur]);
        // Find the following segment
        cur = face.findNext(cur);
        if (i++ > face.msegs.size()) {
            // If we have more iterations than the total number of msegments,
            // then we are stuck in an endless loop.
            ret = false;
            cerr << "Endless loop!\n";
            break;
        }
    } while (cur > 0); // We started with msegment 0, so stop here.
    
    if (cur == -1) { // We didn't find a following msegment, this is an error
        ret = false;
        cerr << "Error: cycle incomplete!\n";
    } else if (cur == -2) {
         // findNext returns -2 if two matching msegments were found
        ret = false;
        cerr << "Error: cycle ambiguous!\n";
    } else if (cur < 0) {
        // Should never happen
        ret = false;
        cerr << "Error: unknown error in cycle!\n";
    }

    if (s2.size() != face.msegs.size()) {
        // If the sorted cycle has less msegments than the original list,
        // then we didn't use all of them. This is an error.
        ret = false;
        cerr << "Unused segments! " << "has " << face.msegs.size() <<
                " used " << s2.size() << "\n";
    } else {
        face.msegs = s2;
    }
    
    return ret;
}

/*
   1.3 Check
   Performs several sanity-checks on this object

*/

bool MFace::Check() {
    bool ret = true;
    
    // Checking Border-Regions
    
    CreateBorderFace(true);
    CreateBorderFace(false);
    
    for (unsigned int i = 0; i < holes.size(); i++) {
        MFace h1(holes[i]);
        for (unsigned int j = 0; j < holes.size(); j++) {
            if (holes[i].intersects(holes[j], false, false))
                ret = false;
        }
    }
    
    if (!SortCycle()) {
        ret = false;
    }

    if (face.intersects(face, false, true)) {
        cerr << "Intersection!\n";
        ret = false;
    }
    
    if (!ret) {
        cerr << "Error with MFace " << this->ToString() << "\n";
    }
    
    MFaces mfs(*this);
    Interval<Instant> mainIv(0.0,100.0,true,true);
    bool correct = true;
    ListExpr le, err = nl->Empty();
    
    ListExpr c = ToListExpr();
    ListExpr interval = 
            nl->OneElemList(nl->StringAtom(mainIv.start.ToString(false), true));
    le = interval;
    le = nl->Append(le, nl->StringAtom(mainIv.end.ToString(false), true));
    le = nl->Append(le, nl->BoolAtom(mainIv.lc));
    le = nl->Append(le, nl->BoolAtom(mainIv.rc));
    ListExpr top = nl->OneElemList(interval);
    nl->Append(top, nl->OneElemList(c));
    ListExpr mreg = nl->OneElemList(top);
    
    InMRegion(nl->Empty(), mreg, 0, err, correct);
    if (!correct) {
        nl->WriteListExpr(mreg);
        cerr << "MRegion import failed!\n";
        ret = false;
    }
    assert(ret);

    return ret;
}

/*
   1.4 AddConcavity
 
   Add a new concavity or hole to this MFace-Object.
   The cycle will be integrated as a Concavity or hole when the function
   MergeConcavities is called afterwards.
 
*/
void MFace::AddConcavity(MSegs c) {
    cvs.push_back(c);
}

static ListExpr CycleToListExpr(MSegs face);

/*
 1.5 MergeConcavities
 
 Merge the objects in the concavities-list into the current cycle.
 These will either be integrated into the cycle if possible, or otherwise be
 added as a hole.
 
*/
void MFace::MergeConcavities() {
    Check();
    for (unsigned int i = 0; i < cvs.size(); i++) {
        if (face.MergeConcavity(cvs[i])) {
            // Merging the concavity into the cycle was successful.
            PrintMRegionListExpr();
        } else {
            // Merging the concavity into the cycle was not successful, add
            // this cycle to the list of holes.
            holes.push_back(cvs[i]);
            
            // If the hole was not a real hole but a concavity in the source- or
            // destination region, we have to create a start and/or end region.
            if (!cvs[i].sreg.ishole)
                needStartRegion = true;
            if (!cvs[i].dreg.ishole)
                needEndRegion = true;
        }
    }
    
    // All Concavities have been handled, clear the list.
    cvs.erase(cvs.begin(), cvs.end());
}

/*
 1.6 Create a URegion-Object from this MFace
 
 As the time of this writing, the URegion-Constructor didn't work properly, so
 the resulting URegion would be broken.
 
*/
URegion MFace::ToURegion(Interval<Instant> iv, int facenr) {
    // Create the MSegmentData-objects from this cycle
    vector<MSegmentData> ms = face.ToMSegmentData(facenr, 0, 0);
    for (unsigned int i = 0; i < holes.size(); i++) {
        vector<MSegmentData> h = holes[i].ToMSegmentData(facenr, i + 1,
                ms.size());
        ms.insert(ms.end(), h.begin(), h.end());
    }
    // Finally, construct the URegion
    URegion ret(ms, iv);

    return ret;
}

/*
 1.7 CycleToListExpr takes a cycle of MSegs and constructs a NestedList
 suitable to be integrated into the NestedList-Representation of a URegion
 or MRegion.
 
*/
static ListExpr CycleToListExpr(MSegs face) {
    ListExpr le, le2;
    int first, cur;
    first = cur = 0;

    ListExpr c = nl->OneElemList(nl->RealAtom(face.msegs[cur].ie.x));
    le = nl->Append(c, nl->RealAtom(face.msegs[cur].ie.y));
    le = nl->Append(le, nl->RealAtom(face.msegs[cur].fe.x));
    le = nl->Append(le, nl->RealAtom(face.msegs[cur].fe.y));
    ListExpr cy = nl->OneElemList(c);
    le2 = cy;
    cur = face.findNext(cur);
    while (cur != first) {
        assert(cur >= 0);
        c = nl->OneElemList(nl->RealAtom(face.msegs[cur].ie.x));
        le = nl->Append(c, nl->RealAtom(face.msegs[cur].ie.y));
        le = nl->Append(le, nl->RealAtom(face.msegs[cur].fe.x));
        le = nl->Append(le, nl->RealAtom(face.msegs[cur].fe.y));
        le2 = nl->Append(le2, c);
        cur = face.findNext(cur);
    }

    return cy;
}

/*
 1.8 ToListExpr converts this face and its holes to a NestedList-Expression
 suitable to be embedded in a uregion- or mregion-NestedList
 
 
*/
ListExpr MFace::ToListExpr() {
    ListExpr ret = nl->OneElemList(CycleToListExpr(face));
    ListExpr le = ret;
    for (unsigned int i = 0; i < holes.size(); i++) {
        le = nl->Append(le, CycleToListExpr(holes[i]));
    }

    return ret;
}

/*
 1.9 PrintMRegionListExpr is used for debugging purposes and prints an
 OBJECT-representation of this MFace suitable to be loaded with
 "restore" into the database
 
*/
void MFace::PrintMRegionListExpr() {
    cerr << "(OBJECT mr () mregion ( ( "
            "(\"2013-01-01\" \"2013-01-02\" TRUE TRUE)"
            << nl->ToString(ToListExpr()) << ") ) )";
}

/*
 1.10 divide is used to create a MFace from this MFace over a part of the whole
 time-interval.
 For example: divide(0.0, 0.5) creates an MFace over the first half of the
 original time-interval.
 
*/
MFace MFace::divide(double start, double end) {
    MFace ret(face.divide(start, end));

    for (unsigned int i = 0; i < holes.size(); i++) {
        MSegs m = holes[i].divide(start, end);
        ret.AddConcavity(m);
    }
    ret.MergeConcavities();

    return ret;
}

/*
  1.11 ToString creates a textual representation of this face including its
  holes.

*/
string MFace::ToString() {
    std::ostringstream ss;

    ss << "Face:\n";
    ss << face.ToString();
    for (unsigned int i = 0; i < holes.size(); i++) {
        ss << "Hole " << i << ":\n"
                << holes[i].ToString();
    }
    ss << "\n";

    return ss.str();
}


/*
 1.12 CreateBorderFace is used to reconstruct a Face from this MFace.
 
 If the parameter ~src~ is TRUE, then the face is created from the initial
 segments of the MSegs (thus representing the state at the start of the
 time-interval), otherwise from the final segments.
 
*/
Face MFace::CreateBorderFace(bool src) {
    assert(SortCycle());

    Face ret = face.CreateBorderFace(src);

    for (unsigned int h = 0; h < holes.size(); h++) {
        Face hole = holes[h].CreateBorderFace(src);
        
        // AddHole also merges concavities if segments overlap
        ret.AddHole(hole);
    }

    return ret;
}
